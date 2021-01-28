#include "Graphics.h"
#include "Material.h"
#include "Scene.h"
#include "SceneVisitor.h"
#include "Managers/TextureManager.h"
#include "API/DX12/DXException.h"
#include "Core/Logger.h"
#include "Graphics/API/DX12/CommandQueue.h"
#include "Graphics/API/DX12/CommandList.h"
#include "Graphics/API/DX12/RootSignature.h"
#include "API/DX12/Device.h"
#include "API/DX12/Swapchain.h"
#include "API/DX12/GeometryGenerator.h"
#include "API/DX12/Mesh.h"
#include "API/DX12/RenderTarget.h"
#include "API/DX12/Texture.h"
#include "API/DX12/ShaderResourceView.h"
#include "Core/Math/Math.h"
#include "API/DX12/VertexTypes.h"
#include <chrono>
#include <cstdlib>

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

static constexpr auto g_longMax = std::numeric_limits<long>::max();

struct Mat {
    dx::XMMATRIX ModelMatrix;
    dx::XMMATRIX ModelViewMatrix;
    dx::XMMATRIX InverseTransposeModelViewMatrix;
    dx::XMMATRIX ModelViewProjectionMatrix;
};

struct LightProperties {
    uint32_t numPointLights;
    uint32_t numSpotLights;
};

enum RootParameters {
    MatricesCB,
    MaterialCB,
    LightPropertiesCB,
    PointLights,
    SpotLights,
    Textures,
    NumRootParameters
};

dx::XMMATRIX XM_CALLCONV LookAtMatrix(dx::FXMVECTOR Position, dx::FXMVECTOR Direction, dx::FXMVECTOR Up)
{
    assert(!dx::XMVector3Equal(Direction, dx::XMVectorZero()));
    assert(!dx::XMVector3IsInfinite(Direction));
    assert(!dx::XMVector3Equal(Up, dx::XMVectorZero()));
    assert(!dx::XMVector3IsInfinite(Up));

    dx::XMVECTOR R2 = dx::XMVector3Normalize(Direction);

    dx::XMVECTOR R0 = dx::XMVector3Cross(Up, R2);
    R0 = dx::XMVector3Normalize(R0);

    dx::XMVECTOR R1 = dx::XMVector3Cross(R2, R0);

    dx::XMMATRIX M(R0, R1, R2, Position);

    return M;
}

void XM_CALLCONV ComputeMatrices(dx::FXMMATRIX model, dx::CXMMATRIX view, dx::CXMMATRIX viewProjection, Mat& mat)
{
    mat.ModelMatrix = model;
    mat.ModelViewMatrix = model * view;
    mat.InverseTransposeModelViewMatrix = dx::XMMatrixTranspose(dx::XMMatrixInverse(nullptr, mat.ModelViewMatrix));
    mat.ModelViewProjectionMatrix = model * viewProjection;
}

Cyrex::Graphics::Graphics()
    :
    m_scissorRect(CD3DX12_RECT(0, 0, g_longMax, g_longMax))  
{
    dx::XMVECTOR cameraPos    = dx::XMVectorSet(0, 5.0f, -20, 1);
    dx::XMVECTOR cameraTarget = dx::XMVectorSet(0, 5, 0, 1);
    dx::XMVECTOR cameraUp     = dx::XMVectorSet(0, 1, 0, 0);

    m_camera.SetLookAt(cameraPos, cameraTarget, cameraUp);

    m_cameraData = new CameraData;
   
    m_cameraData->InitialCamPos = m_camera.GetTranslation();
    m_cameraData->InitialCamRot = m_camera.GetRotation();
}

Cyrex::Graphics::~Graphics() {
    delete m_cameraData;
    UnLoadContent();
}

void Cyrex::Graphics::Initialize(uint32_t width, uint32_t height) {
    //Check for DirectX Math suport
    if (!DirectX::XMVerifyCPUSupport()) {
        crxlog::err("No support for DirectX Math found");
        m_isIntialized = false;
        return;
    }

    m_device = Device::Create();
    m_swapChain = m_device->CreateSwapChain(m_hWnd, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_tearingSupported = m_swapChain->IsTearingSupported();

    m_swapChain->SetVsync(m_vsync);
    
    m_clientWidth = width;
    m_clientHeight = height;
   
    LoadContent();
    Resize(width, height);
    m_isIntialized = true;
}

void Cyrex::Graphics::Update() noexcept {
    using namespace DirectX;

    static uint64_t frameCount = 0;

    m_timer.Tick();
    frameCount++;

    if (m_timer.GetElapsedSeconds() > 1.0) {
        const auto fps = frameCount / m_timer.GetElapsedSeconds();
        crxlog::normal(fps, " fps");
        frameCount = 0;
        m_timer.ResetElapsedTime();
    }

    m_swapChain->WaitForSwapChain();

    float speedMultiPlier = m_cameraControls.Sneak ? 4.0f : 16.0f;

    XMVECTOR translate = XMVectorSet(
        0,
        0.0f,
        0.0f,
        1.0f) * speedMultiPlier * static_cast<float>(m_timer.GetDeltaSeconds());

    XMVECTOR pan = XMVectorSet(0.0f, -0.0f, 0.0f, 1.0f) 
                   * speedMultiPlier * static_cast<float>(m_timer.GetDeltaSeconds());

    m_camera.Translate(translate, Space::Local);
    m_camera.Translate(pan, Space::Local);

   /* XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(
        XMConvertToRadians(m_cameraControls.Pitch), 
        XMConvertToRadians(m_cameraControls.Yaw), 0.0f);

    m_camera.SetRotation(rotation);*/

    XMMATRIX view = m_camera.GetView();

    constexpr int numPointLights = 4;
    constexpr int numSpotLights  = 4;

    static const std::array lightColors = { Colors::White, Colors::Orange, Colors::Yellow, Colors::Green,
                                            Colors::Blue, Colors::Indigo, Colors::Violet, Colors::White };

    static float lighAnimationTime = 0.0f;

    if (m_animateLights) {
        lighAnimationTime += static_cast<float>(m_timer.GetDeltaSeconds()) * 0.5f * Math::MathConstants::pi_float;
    }

    constexpr float radius = 8.0f;
    constexpr float pointLightoffset = 2.0f * Math::MathConstants::pi_float / numPointLights;
    constexpr float spotLightOffset = pointLightoffset + (pointLightoffset / 2.0f);

    m_pointLights.resize(numPointLights);
    for (int i = 0; i < numPointLights; i++) {
        PointLight& light = m_pointLights[i];

        light.WorldSpacePosition = { static_cast<float>(std::sin(lighAnimationTime + pointLightoffset * i)) * radius, 9.0f,
                                     static_cast<float>(std::cos(lighAnimationTime + pointLightoffset * i)) * radius, 1.0f };

        XMVECTOR worldSpacePos = XMLoadFloat4(&light.WorldSpacePosition);
        XMVECTOR viewSpacePos  = XMVector3TransformCoord(worldSpacePos, view);
        XMStoreFloat4(&light.ViewSpacePosition, viewSpacePos);

        light.Color                = XMFLOAT4(lightColors[i]);
        light.ConstantAttenuation  = 1.0f;
        light.LinearAttenuation    = 0.08f;
        light.QuadraticAttenuation = 0.0f;
    }

    m_spotLights.resize(numSpotLights);
    for (int i = 0; i < numSpotLights; i++) {
        SpotLight& light = m_spotLights[i];

        light.WorldSpacePosition = { static_cast<float>(std::sin(lighAnimationTime + pointLightoffset * i + spotLightOffset)) * radius, 9.0f,
                                     static_cast<float>(std::cos(lighAnimationTime + pointLightoffset * i + spotLightOffset)) * radius, 1.0f };

        XMVECTOR worldSpacePos = XMLoadFloat4(&light.WorldSpacePosition);
        XMVECTOR viewSpacePos  = XMVector3TransformCoord(worldSpacePos, view);
        XMStoreFloat4(&light.ViewSpacePosition, viewSpacePos);

        light.Color                = XMFLOAT4(lightColors[numPointLights + i]);
        light.SpotAngle            = XMConvertToRadians(45.0f);
        light.ConstantAttenuation  = 1.0f;
        light.LinearAttenuation    = 0.08f;
        light.QuadraticAttenuation = 0.0f;
    }
}

void Cyrex::Graphics::LoadContent() {
    auto& commandQueue = m_device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto commandList   = commandQueue.GetCommandList();

    //Create some scenes
    m_cube   = GeometryGenerator::CreateCube(commandList);
    m_sphere = GeometryGenerator::CreateSphere(commandList);
    m_cone   = GeometryGenerator::CreateCone(commandList);
    m_torus  = GeometryGenerator::CreateTorus(commandList);
    m_plane  = GeometryGenerator::CreatePlane(commandList);

    // Load some textures
    m_defaultTexture  = TextureManager::LoadTextureFromFile(*commandList, L"Resources/Textures/DefaultWhite.bmp", true);
    m_directXTexture  = TextureManager::LoadTextureFromFile(*commandList, L"Resources/Textures/Directx9.png", true);
    m_earthTexture    = TextureManager::LoadTextureFromFile(*commandList, L"Resources/Textures/earth.dds", true);
    m_monaLisaTexture = TextureManager::LoadTextureFromFile(*commandList, L"Resources/Textures/Mona_Lisa.jpg", true);

    commandQueue.ExecuteCommandList(commandList);

    // Load the shaders.
    wrl::ComPtr<ID3DBlob> vertexShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(LR"(Graphics\Shaders\Compiled\VertexShader.cso)", &vertexShaderBlob));

    wrl::ComPtr<ID3DBlob> pixelShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(LR"(Graphics\Shaders\Compiled\PixelShader.cso)", &pixelShaderBlob));

    wrl::ComPtr<ID3DBlob> unlitPixelShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(LR"(Graphics\Shaders\Compiled\UnlitPixelShader.cso)", &unlitPixelShaderBlob));

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    CD3DX12_DESCRIPTOR_RANGE1 descriptorRage(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

    CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters]{};

    rootParameters[RootParameters::MatricesCB].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[RootParameters::MaterialCB].InitAsConstantBufferView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

    rootParameters[RootParameters::LightPropertiesCB].InitAsConstants(sizeof(LightProperties) / 4, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::PointLights].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

    rootParameters[RootParameters::SpotLights].InitAsShaderResourceView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::Textures].InitAsDescriptorTable(1, &descriptorRage, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
    CD3DX12_STATIC_SAMPLER_DESC anisotropicSampler(0, D3D12_FILTER_ANISOTROPIC);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(RootParameters::NumRootParameters, rootParameters, 1, &linearRepeatSampler, rootSignatureFlags);

    m_rootSignature = m_device->CreateRootSignature(rootSignatureDescription.Desc_1_1);

    // Setup the pipeline state
    struct PipeLineStateStream {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          Inputlayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
    } pipelineStateStream;

    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

    // Check the multisample quality level for the backbuffer
    DXGI_SAMPLE_DESC sampleDesc = m_device->GetMultisampleQualityLevels(backBufferFormat);

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = backBufferFormat;

    pipelineStateStream.pRootSignature        = m_rootSignature->GetRootSignature().Get();
    pipelineStateStream.Inputlayout           = VertexPositionNormalTangentBitangentTexture::InputLayout;
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.DSVFormat             = depthBufferFormat;
    pipelineStateStream.RTVFormats            = rtvFormats;
    pipelineStateStream.SampleDesc            = sampleDesc;

    m_pipelineState = m_device->CreatePipelineStateObject(pipelineStateStream);

    // For the unlit PSO, only the pixel shader is different.
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(unlitPixelShaderBlob.Get());
    m_unlitPipelineState = m_device->CreatePipelineStateObject(pipelineStateStream);

    // Create an off-screen render target with a single color buffer and a depth buffer.
    auto colorDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        backBufferFormat,
        m_clientWidth,
        m_clientHeight,
        1,
        1,
        sampleDesc.Count,
        sampleDesc.Quality,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    auto color = DirectX::Colors::LightSteelBlue.f;
    D3D12_CLEAR_VALUE colorClearValue;
    colorClearValue.Format = colorDesc.Format;
    colorClearValue.Color[0] = color[0];
    colorClearValue.Color[1] = color[1];
    colorClearValue.Color[2] = color[2];
    colorClearValue.Color[3] = color[3];

    auto colorTexture = m_device->CreateTexture(colorDesc, &colorClearValue);
    colorTexture->SetName(L"Color Render Target");

    // Create a depth buffer.
    auto depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(depthBufferFormat,
        m_clientWidth,
        m_clientHeight,
        1,
        1,
        sampleDesc.Count,
        sampleDesc.Quality,
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    D3D12_CLEAR_VALUE depthClearValue;
    depthClearValue.Format = depthDesc.Format;
    depthClearValue.DepthStencil = { 1.0f, 0 };

    auto depthTexture = m_device->CreateTexture(depthDesc, &depthClearValue);
    depthTexture->SetName(L"Depth Render Target");

    // Attach the textures to the render target.
    m_renderTarget.AttachTexture(AttachmentPoint::Color0, colorTexture);
    m_renderTarget.AttachTexture(AttachmentPoint::DepthStencil, depthTexture);

    commandQueue.Flush();
}

void Cyrex::Graphics::UnLoadContent() noexcept {
    m_cube.reset();
    m_sphere.reset();
    m_cone.reset();
    m_torus.reset();
    m_plane.reset();

    m_defaultTexture.reset();
    m_directXTexture.reset();
    m_earthTexture.reset();
    m_monaLisaTexture.reset();

    m_renderTarget.Reset();
    m_rootSignature.reset();
    m_pipelineState.reset();
    m_unlitPipelineState.reset();

    m_swapChain.reset();
    m_device.reset();
}

void Cyrex::Graphics::Render() {
    using namespace DirectX;

    auto& commandQueue = m_device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto commandList = commandQueue.GetCommandList();

    //Create the scene visitor
    SceneVisitor visitor(*commandList);

    // Clear the render targets.
    {
        TextureManager::ClearTexture(*commandList, m_renderTarget.GetTexture(AttachmentPoint::Color0), DirectX::Colors::LightSteelBlue);
        TextureManager::ClearDepthStencilTexture(*commandList, m_renderTarget.GetTexture(AttachmentPoint::DepthStencil), D3D12_CLEAR_FLAG_DEPTH);
    }

    commandList->SetPipelineState(m_pipelineState);
    commandList->SetGraphicsRootSignature(m_rootSignature);

    LightProperties lightProps;
    lightProps.numPointLights = static_cast<uint32_t>(m_pointLights.size());
    lightProps.numSpotLights  = static_cast<uint32_t>(m_spotLights.size());

    commandList->SetGraphics32BitConstants(RootParameters::LightPropertiesCB, lightProps);
    commandList->SetGraphicsDynamicStructuredBuffer(RootParameters::PointLights, m_pointLights);
    commandList->SetGraphicsDynamicStructuredBuffer(RootParameters::SpotLights, m_spotLights);

    commandList->SetViewport(m_viewport);
    commandList->SetScissorRect(m_scissorRect);

    commandList->SetRenderTarget(m_renderTarget);

    // Draw the earth sphere
    XMMATRIX translationMatrix = XMMatrixTranslation(-4.0f, 2.0f, -4.0f);
    XMMATRIX rotationMatrix    = XMMatrixIdentity();
    XMMATRIX scaleMatrix       = XMMatrixScaling(4.0f, 4.0f, 4.0f);
    XMMATRIX worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;
    XMMATRIX viewMatrix        = m_camera.GetView();
    XMMATRIX viewProjMatrix    = viewMatrix * m_camera.GetProj();

    Mat matrices;
    ComputeMatrices(worldMatrix, viewMatrix, viewProjMatrix, matrices);

    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::White);
    commandList->SetShaderResourceView(
        RootParameters::Textures, 
        0, 
        m_earthTexture,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    m_sphere->Accept(visitor);


    //Draw a cube
    translationMatrix = XMMatrixTranslation(4.0f, 4.0f, 4.0f);
    rotationMatrix    = XMMatrixRotationY(XMConvertToRadians(45.0f));
    scaleMatrix       = XMMatrixScaling(4.0f, 8.0f, 4.0f);
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices(worldMatrix, viewMatrix, viewProjMatrix, matrices);

    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::White);
    commandList->SetShaderResourceView(
        RootParameters::Textures, 
        0, 
        m_monaLisaTexture,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    m_cube->Accept(visitor);

    // Draw a torus
    translationMatrix = XMMatrixTranslation(4.0f, 0.6f, -4.0f);
    rotationMatrix    = XMMatrixRotationY(XMConvertToRadians(45.0f));
    scaleMatrix       = XMMatrixScaling(4.0f, 4.0f, 4.0f);
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices(worldMatrix, viewMatrix, viewProjMatrix, matrices);

    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::YellowPlastic);
    commandList->SetShaderResourceView(RootParameters::Textures, 0, m_defaultTexture,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    m_torus->Accept(visitor);

    // Floor plane.
    float scalePlane = 20.0f;
    float translateOffset = scalePlane / 2.0f;

    translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    rotationMatrix    = XMMatrixIdentity();
    scaleMatrix       = XMMatrixScaling(scalePlane, 1.0f, scalePlane);
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices(worldMatrix, viewMatrix, viewProjMatrix, matrices);

    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::White);
    commandList->SetShaderResourceView(
        RootParameters::Textures, 
        0,
        m_directXTexture,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    m_plane->Accept(visitor);

    // Back wall
    translationMatrix = XMMatrixTranslation(0, translateOffset, translateOffset);
    rotationMatrix    = XMMatrixRotationX(XMConvertToRadians(-90));
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices(worldMatrix, viewMatrix, viewProjMatrix, matrices);

    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);

    m_plane->Accept(visitor);

    // Ceiling plane
    translationMatrix = XMMatrixTranslation(0, translateOffset * 2.0f, 0);
    rotationMatrix    = XMMatrixRotationX(XMConvertToRadians(180));
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices(worldMatrix, viewMatrix, viewProjMatrix, matrices);

    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);

    m_plane->Accept(visitor);

    // Front wall
    translationMatrix = XMMatrixTranslation(0, translateOffset, -translateOffset);
    rotationMatrix    = XMMatrixRotationX(XMConvertToRadians(90));
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices(worldMatrix, viewMatrix, viewProjMatrix, matrices);

    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);

    m_plane->Accept(visitor);

    // Left wall
    translationMatrix = XMMatrixTranslation(-translateOffset, translateOffset, 0);
    rotationMatrix    = XMMatrixRotationX(XMConvertToRadians(-90)) * XMMatrixRotationY(XMConvertToRadians(-90));
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices(worldMatrix, viewMatrix, viewProjMatrix, matrices);

    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::Red);
    commandList->SetShaderResourceView(RootParameters::Textures, 0, m_defaultTexture,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    m_plane->Accept(visitor);

    // Right wall
    translationMatrix = XMMatrixTranslation(translateOffset, translateOffset, 0);
    rotationMatrix    = XMMatrixRotationX(XMConvertToRadians(-90)) * XMMatrixRotationY(XMConvertToRadians(90));
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices(worldMatrix, viewMatrix, viewProjMatrix, matrices);

    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
    commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::Blue);
    m_plane->Accept(visitor);

    commandList->SetPipelineState(m_unlitPipelineState);

    MaterialProperties lightMaterial = Material::Zero;;

    for (const auto& pointLight : m_pointLights) {
        lightMaterial.Emissive = pointLight.Color;
        dx::XMVECTOR lightPos  = dx::XMLoadFloat4(&pointLight.WorldSpacePosition);
        worldMatrix            = dx::XMMatrixTranslationFromVector(lightPos);

        ComputeMatrices(worldMatrix, viewMatrix, viewProjMatrix, matrices);

        commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
        commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, lightMaterial);

        m_sphere->Accept(visitor);;
    }

    for (const auto& spotLight : m_spotLights) {
        lightMaterial.Emissive  = spotLight.Color;
        dx::XMVECTOR lightPos   = dx::XMLoadFloat4(&spotLight.WorldSpacePosition);
        dx::XMVECTOR lightDir   = dx::XMLoadFloat4(&spotLight.WorldSpaceDirection);
        dx::XMVECTOR up         = dx::XMVectorSet(0, 1, 0, 0);

        rotationMatrix          = dx::XMMatrixRotationX(dx::XMConvertToRadians(-45.0f));
        worldMatrix             = rotationMatrix * LookAtMatrix(lightPos, lightDir, up);

        ComputeMatrices(worldMatrix, viewMatrix, viewProjMatrix, matrices);

        commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
        commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, lightMaterial);

        m_cone->Accept(visitor);
    }

    auto& swapChainRT        = m_swapChain->GetRenderTarget();
    auto swapChainBackBuffer = swapChainRT.GetTexture(AttachmentPoint::Color0);
    auto msaaRenderTarget    = m_renderTarget.GetTexture(AttachmentPoint::Color0);

    commandList->ResolveSubresource(swapChainBackBuffer, msaaRenderTarget);

    commandQueue.ExecuteCommandList(commandList);

    m_swapChain->Present();
}

void Cyrex::Graphics::Resize(uint32_t width, uint32_t height) {
    m_clientWidth  = std::max(1u, width);
    m_clientHeight = std::max(1u, height);

    m_swapChain->Resize(m_clientWidth, m_clientHeight);

    float aspectRatio = m_clientWidth / static_cast<float>(m_clientHeight);
    m_camera.SetProj(45.0f, aspectRatio, 0.1f, 100.0f);

    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_clientWidth), static_cast<float>(m_clientHeight));

    m_renderTarget.Resize(m_clientWidth, m_clientHeight);
}

void Cyrex::Graphics::ToggleVsync() {
    m_swapChain->ToggleVsync();
}

void Cyrex::Graphics::OnMouseWheel(float delta) noexcept {
    float fov = m_camera.GetFov();

    fov -= delta;
    fov = std::clamp(fov, 12.0f, 90.0f);

    m_camera.SetFov(fov);

    crxlog::info("Fov: ", fov);
}

void Cyrex::Graphics::OnMouseMoved(int dx, int dy) noexcept {
    //TODO: implement this
}
