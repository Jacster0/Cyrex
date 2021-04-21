#include "D3D12Layer.h"

#include "Graphics/API/DX12/CommandList.h"
#include "Graphics/API/DX12/CommandQueue.h"
#include "Graphics/API/DX12/Device.h"
#include "Graphics/API/DX12/RenderTarget.h"
#include "Graphics/API/DX12/RootSignature.h"
#include "Graphics/API/DX12/ShaderResourceView.h"
#include "Graphics/API/DX12/Swapchain.h"
#include "Graphics/API/DX12/Texture.h"
#include "Graphics/API/DX12/DXException.h"
#include "Graphics/API/DX12/PipelineStateObject.h"
#include "Graphics/Viewport.h"

#include "Core/Logger.h"
#include "Core/Utils/TextureUtils.h"
#include "Core/Math/Matrix.h"
#include "Core/Math/Rectangle.h"

#include "ImGui/imgui_impl_win32.h"
#include "Extern/DirectXTex/DirectXTex/DirectXTex.h"

#include <d3dcompiler.h>

using namespace Cyrex;
using namespace Cyrex::Math;

enum RootParameters {
    MatrixCB,         //cbuffer vertexBuffer : register(b0)
    FontTexture,      //Texture2D texture0   : register(t0);
    NumRootParameters,     
};

D3D12Layer::D3D12Layer(Device& device, SwapChain& swapChain, HWND hWnd) noexcept
    :
    EditorLayer(device, swapChain, hWnd)
{}

void D3D12Layer::Attach() {
    //Load the shaders
    ThrowIfFailed(D3DReadFileToBlob(L"Graphics/Shaders/Compiled/ImGuiVS.cso", &m_vertexShader));
    ThrowIfFailed(D3DReadFileToBlob(L"Graphics/Shaders/Compiled/ImGuiPS.cso", &m_pixelShader));

    //Setup ImGui context
    IMGUI_CHECKVERSION();
    m_imGuiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_imGuiContext);

    if (!ImGui_ImplWin32_Init(m_hWnd)) {
        crxlog::err("Could not initialize ImGui platform backend");
        throw std::exception("Failed to initialize ImGui");
    }

    ImGuiIO& io = GetIO();
    io.ConfigFlags         |= m_configFlags;
    io.FontGlobalScale      = GetDpiForWindow(m_hWnd) / 96.0f;
    io.FontAllowUserScaling = true;

    BuildFontTexture();
    //Create the root signature for the ImGUI shaders
    CreateRootSignature();
    //Create the pipeline state object
    CreatePSO();
}

void D3D12Layer::Detach() noexcept {
    ImGui::EndFrame();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext(m_imGuiContext);
    m_imGuiContext = nullptr;
}

void D3D12Layer::Begin() noexcept {
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void D3D12Layer::End(CommandList& cmdList) noexcept {
    ImGui::Render();

    ImGuiIO& io = GetIO();
    ImDrawData* drawData = ImGui::GetDrawData();

    // Avoid rendering when minimized
    const int width  = static_cast<int>(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    const int height = static_cast<int>(drawData->DisplaySize.y * drawData->FramebufferScale.y);

    if (width <= 0 || height <= 0 || drawData->TotalVtxCount == 0) {
        return;
    }
       
    //Check if there is anything to render
    if (!drawData || drawData->CmdListsCount == 0) {
        crxlog::debug("No Gui to render");
        return;
    }

    const auto& renderTarget = m_swapChain.GetRenderTarget();

    cmdList.SetPipelineState(m_pipelineState);
    cmdList.SetGraphicsRootSignature(m_rootSignature);
    cmdList.SetRenderTarget(renderTarget);

    // Set root arguments.
    const float L = drawData->DisplayPos.x;
    const float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
    const float T = drawData->DisplayPos.y;
    const float B = drawData->DisplayPos.y + drawData->DisplaySize.y;

    const Matrix mvp
    (
         2.0f / (R - L), 0.0f, 0.0f, 0.0f ,
         0.0f, 2.0f / (T - B), 0.0f, 0.0f ,
         0.0f, 0.0f, 0.5f, 0.0f ,
         (R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f
    );

    cmdList.SetGraphics32BitConstants(RootParameters::MatrixCB, mvp);
    cmdList.SetShaderResourceView(
        RootParameters::FontTexture,
        0,
        m_fontSRV,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  

    Viewport viewport { 
        .Width = drawData->DisplaySize.x, 
        .Height = drawData->DisplaySize.y, 
        .MinDepth = 0.0f, 
        .MaxDepth = 1.0f 
    };

    cmdList.SetViewport(viewport);
    cmdList.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const DXGI_FORMAT indexFormat = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    // It may happen that ImGui doesn't actually render anything. In this case,
    // any pending resource barriers in the commandList will not be flushed (since
    // resource barriers are only flushed when a draw command is executed).
    // In that case, manually flushing the resource barriers will ensure that
    // they are properly flushed before exiting this function.
    cmdList.FlushResourceBarriers();

    for (int i = 0; i < drawData->CmdListsCount; ++i) {
        const ImDrawList* drawList = drawData->CmdLists[i];

        cmdList.SetDynamicVertexBuffer(0, drawList->VtxBuffer.size(), sizeof(ImDrawVert), drawList->VtxBuffer.Data);
        cmdList.SetDynamicIndexBuffer(drawList->IdxBuffer.size(), indexFormat, drawList->IdxBuffer.Data);

        int indexOffset = 0;
        for (int j = 0; j < drawList->CmdBuffer.size(); ++j) {
            const ImDrawCmd& drawCmd = drawList->CmdBuffer[j];

            if (drawCmd.UserCallback) {
                drawCmd.UserCallback(drawList, &drawCmd);
            }

            else {
                ImVec4 clipRect = drawCmd.ClipRect;

                Math::Rectangle scissorRect(
                    clipRect.x - drawData->DisplayPos.x,
                    clipRect.y - drawData->DisplayPos.y,
                    clipRect.z - drawData->DisplayPos.x,
                    clipRect.w - drawData->DisplayPos.y);
            
                cmdList.SetScissorRect(scissorRect);
                cmdList.DrawIndexed(drawCmd.ElemCount, 1, indexOffset);
            }
            indexOffset += drawCmd.ElemCount;
        }
    }

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void D3D12Layer::CreateRootSignature() noexcept {
    std::array<CD3DX12_ROOT_PARAMETER1, RootParameters::NumRootParameters> rootParameters{};

    CD3DX12_DESCRIPTOR_RANGE1 descriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[RootParameters::FontTexture].InitAsDescriptorTable(1, &descriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);


    rootParameters[MatrixCB].InitAsConstants(
        sizeof(Matrix) / 4,
        0,
        D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[FontTexture].InitAsDescriptorTable(
        1,
        &descriptorRange,
        D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_STATIC_SAMPLER_DESC samplerDescription(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(
        rootParameters.size(),
        rootParameters.data(),
        1,
        &samplerDescription,
        m_rootSignatureFlags);

    m_rootSignature = std::make_shared<RootSignature>(m_device, rootSignatureDescription.Desc_1_1);
}

void D3D12Layer::CreatePSO() noexcept {
    const auto& renderTarget = m_swapChain.GetRenderTarget();

    constexpr auto inputLayout = std::to_array<D3D12_INPUT_ELEMENT_DESC>(
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, offsetof(ImDrawVert, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, offsetof(ImDrawVert, uv),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(ImDrawVert, col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        }
    );

    D3D12_BLEND_DESC blendDesc                      = {};
    blendDesc.RenderTarget[0].BlendEnable           = true;
    blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_RASTERIZER_DESC rasterizerDesc  = {};
    rasterizerDesc.FillMode               = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode               = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FrontCounterClockwise  = false;
    rasterizerDesc.DepthBias              = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp         = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias   = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable        = true;
    rasterizerDesc.MultisampleEnable      = false;
    rasterizerDesc.AntialiasedLineEnable  = false;
    rasterizerDesc.ForcedSampleCount      = 0;
    rasterizerDesc.ConservativeRaster     = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable              = false;
    depthStencilDesc.StencilEnable            = false;

    //Setup the actual pipeline state
    struct PipelineStateStream {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC            BlendDesc;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL         DepthStencilState;
    } pipelineStateStream;

    pipelineStateStream.pRootSignature        = m_rootSignature->GetRootSignature().Get();
    pipelineStateStream.InputLayout           = { inputLayout.data(), inputLayout.size() };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE(m_vertexShader.Get());
    pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE(m_pixelShader.Get());
    pipelineStateStream.RTVFormats            = renderTarget.GetRenderTargetFormats();
    pipelineStateStream.SampleDesc            = renderTarget.GetSampleDesc();
    pipelineStateStream.BlendDesc             = CD3DX12_BLEND_DESC(blendDesc);
    pipelineStateStream.RasterizerState       = CD3DX12_RASTERIZER_DESC(rasterizerDesc);
    pipelineStateStream.DepthStencilState     = CD3DX12_DEPTH_STENCIL_DESC(depthStencilDesc);

    m_pipelineState = std::make_shared<PipelineStateObject>(m_device, D3D12_PIPELINE_STATE_STREAM_DESC{ sizeof(pipelineStateStream), &pipelineStateStream });
}

void D3D12Layer::BuildFontTexture() noexcept {
    ImGuiIO& io = GetIO();

    //Build texture atlas
    unsigned char* pixelData{ nullptr };
    int width{};
    int height{};

    io.Fonts->GetTexDataAsRGBA32(&pixelData, &width, &height);

    auto& commandQueue = m_device.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto commandList   = commandQueue.GetCommandList();

    auto fontTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);

    m_fontTexture = m_device.CreateTexture(fontTextureDesc);
    m_fontTexture->SetName(L"ImGui Font Texture");
    m_fontSRV = m_device.CreateShaderResourceView(m_fontTexture);

    size_t rowPitch{};
    size_t slicePitch{};

    GetSurfaceInfo(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, &slicePitch, &rowPitch, nullptr);

    D3D12_SUBRESOURCE_DATA subresourceData;
    subresourceData.pData      = pixelData;
    subresourceData.RowPitch   = rowPitch;
    subresourceData.SlicePitch = slicePitch;

    io.Fonts->TexID = static_cast<ImTextureID>(m_fontTexture.get());
    commandList->CopyTextureSubresource(m_fontTexture, 0, 1, &subresourceData);
    commandList->GenerateMips(m_fontTexture);

    commandQueue.ExecuteCommandList(commandList);
}
