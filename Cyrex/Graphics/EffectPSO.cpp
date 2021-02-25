#include "EffectPSO.h"
#include "API/DX12/CommandList.h"
#include "API/DX12/Device.h"
#include "API/DX12/PipelineStateObject.h"
#include "API/DX12/RootSignature.h"
#include "API/DX12/VertexTypes.h"
#include "API/DX12/DXException.h"
#include "Material.h"

#include <d3dcompiler.h>
#include "API/DX12/d3dx12.h"

namespace wrl = Microsoft::WRL;
namespace dx  = DirectX;

Cyrex::EffectPSO::EffectPSO(std::shared_ptr<Device> device, EnableLighting enableLighting, EnableDecal enableDecal)
    :
    m_device(device),
    m_enableLighting(static_cast<bool>(enableLighting)),
    m_enableDecal(static_cast<bool>(enableDecal))
{
    m_pAlignedMVP = new MVP;

    wrl::ComPtr<ID3DBlob> vertexShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"Graphics/Shaders/Compiled/VertexShader.cso", &vertexShaderBlob));

    wrl::ComPtr<ID3DBlob> pixelShaderBlob;

    if (m_enableLighting) {
        if (m_enableDecal) {
            ThrowIfFailed(D3DReadFileToBlob(L"Graphics/Shaders/Compiled/DecalPS.cso", &pixelShaderBlob));
        }
        else {
            ThrowIfFailed(D3DReadFileToBlob(L"Graphics/Shaders/Compiled/LightingPs.cso", &pixelShaderBlob));
        }
    }
    else {
        ThrowIfFailed(D3DReadFileToBlob(L"Graphics/Shaders/Compiled/UnlitPS.cso", &pixelShaderBlob));
    }

    //Create the root signature.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS       |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS     |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    //Descriptor range for the textures
    CD3DX12_DESCRIPTOR_RANGE1 descriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 3);

    CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
    rootParameters[RootParameters::MatricesCB].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[RootParameters::MaterialCB].InitAsConstantBufferView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

    rootParameters[RootParameters::LightPropertiesCB].InitAsConstants(sizeof(LightProperties) / 4, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::PointLights].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::SpotLights].InitAsShaderResourceView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::DirectionalLights].InitAsShaderResourceView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

    rootParameters[RootParameters::Textures].InitAsDescriptorTable(1, &descriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_STATIC_SAMPLER_DESC anisotropicSampler(0, D3D12_FILTER_ANISOTROPIC);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(RootParameters::NumRootParameters, rootParameters, 1, &anisotropicSampler, rootSignatureFlags);

    m_rootSignature = m_device->CreateRootSignature(rootSignatureDescription.Desc_1_1);

    //Set up the pipeline state
    struct PipeLineStateStream {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
    } pipelineStateStream;

    //Create a color buffer with sRGB for gamma correction.
    auto backBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    auto depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

    auto sampleDesc = m_device->GetMultisampleQualityLevels(backBufferFormat);

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets      = 1;
    rtvFormats.RTFormats[0]          = backBufferFormat;

    CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);
    rasterizerState.CullMode = D3D12_CULL_MODE_BACK;

    if (m_enableDecal) {
        //Disable culling on decal geometry
        rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    }

    pipelineStateStream.pRootSignature        = m_rootSignature->GetRootSignature().Get();
    pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.RasterizerState       = rasterizerState;
    pipelineStateStream.InputLayout           = VertexPositionNormalTangentBitangentTexture::InputLayout;
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.DSVFormat             = depthBufferFormat;
    pipelineStateStream.RTVFormats            = rtvFormats;
    pipelineStateStream.SampleDesc            = sampleDesc;

    m_pipelineStateObject = m_device->CreatePipelineStateObject(pipelineStateStream);

    //Create an SRV that can be used to pad unused texture slots.
    D3D12_SHADER_RESOURCE_VIEW_DESC defaultSRV = {};
    defaultSRV.Format                          = DXGI_FORMAT_R8G8B8A8_UNORM;
    defaultSRV.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    defaultSRV.Texture2D.MostDetailedMip       = 0;
    defaultSRV.Texture2D.MipLevels             = 1;
    defaultSRV.Texture2D.PlaneSlice            = 0;
    defaultSRV.Texture2D.ResourceMinLODClamp   = 0;
    defaultSRV.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    m_defaultSRV = m_device->CreateShaderResourceView(nullptr, &defaultSRV);
}

Cyrex::EffectPSO::~EffectPSO() {
    delete m_pAlignedMVP;
}

void Cyrex::EffectPSO::Apply(CommandList& commandList) {
    commandList.SetPipelineState(m_pipelineStateObject);
    commandList.SetGraphicsRootSignature(m_rootSignature);

    if (m_dirtyFlags & DF_Matrices) {
        Matrices matrices;
        matrices.ModelMatrix                     = m_pAlignedMVP->World;
        matrices.ModelViewMatrix                 = matrices.ModelMatrix * m_pAlignedMVP->View;
        matrices.ModelViewProjectionMatrix       = matrices.ModelViewMatrix * m_pAlignedMVP->Projection;
        matrices.InverseTransposeModelViewMatrix = dx::XMMatrixTranspose(dx::XMMatrixInverse(nullptr, matrices.ModelViewMatrix));

        commandList.SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
    }

    if (m_dirtyFlags & DF_Material) {
        if (m_material) {
            const auto& materialProps = m_material->GetMaterialProperties();

            commandList.SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, materialProps);

            using TextureType = Material::TextureType;

            for (uint32_t i = 0; i < TextureType::NumTypes; i++) {
                BindTexture(commandList, i, m_material->GetTexture(static_cast<TextureType>(i)));
            }
        }
    }

    if (m_dirtyFlags & DF_PointLights) {
        commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::PointLights, m_pointLights);
    }

    if (m_dirtyFlags & DF_SpotLights) {
        commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::SpotLights, m_spotLights);
    }

    if (m_dirtyFlags & DF_DirectionalLights) {
        commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::DirectionalLights, m_directionalLights);
    }

    if (m_dirtyFlags & (DF_PointLights | DF_SpotLights | DF_DirectionalLights)) {
        LightProperties lighProps;
        lighProps.NumPointLights       = static_cast<uint32_t>(m_pointLights.size());
        lighProps.NumSpotLights        = static_cast<uint32_t>(m_spotLights.size());
        lighProps.NumDirectionalLights = static_cast<uint32_t>(m_directionalLights.size());

        commandList.SetGraphics32BitConstants(RootParameters::LightPropertiesCB, lighProps);
    }

    m_dirtyFlags = DF_None;
}

inline void Cyrex::EffectPSO::BindTexture(CommandList& commandList, uint32_t offset, const std::shared_ptr<Texture>& texture) {
    if (texture) {
        commandList.SetShaderResourceView(
            RootParameters::Textures, 
            offset, 
            texture, 
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
    else {
        commandList.SetShaderResourceView(
            RootParameters::Textures, 
            offset, 
            m_defaultSRV, 
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}
