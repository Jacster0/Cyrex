#include <chrono>
#include "Graphics.h"
#include "API/DX12/DXException.h"
#include "Core/Logger.h"
#include "Graphics/API/DX12/CommandQueue.h"
#include "Core/Application.h"
#include "Graphics/API/DX12/CommandList.h"
#include "Graphics/API/DX12/RootSignature.h"

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

static constexpr auto g_longMax = std::numeric_limits<long>::max();

static std::array<Cyrex::VertexPosColor, 8> g_Vertices = {
    {
        { dx::XMFLOAT3(-1.0f, -1.0f, -1.0f), dx::XMFLOAT3(0.0f, 0.0f, 0.0f)  },
        { dx::XMFLOAT3(-1.0f,  1.0f, -1.0f), dx::XMFLOAT3(0.0f, 1.0f, 0.0f)  },
        { dx::XMFLOAT3(1.0f,  1.0f, -1.0f),  dx::XMFLOAT3(1.0f, 1.0f, 0.0f)  },
        { dx::XMFLOAT3(1.0f, -1.0f, -1.0f),  dx::XMFLOAT3(1.0f, 0.0f, 0.0f)  },
        { dx::XMFLOAT3(-1.0f, -1.0f,  1.0f), dx::XMFLOAT3(0.0f, 0.0f, 1.0f)  },
        { dx::XMFLOAT3(-1.0f,  1.0f,  1.0f), dx::XMFLOAT3(0.0f, 1.0f, 1.0f)  },
        { dx::XMFLOAT3(1.0f,  1.0f,  1.0f),  dx::XMFLOAT3(1.0f, 1.0f, 1.0f)  },
        { dx::XMFLOAT3(1.0f, -1.0f,  1.0f),  dx::XMFLOAT3(1.0f, 0.0f, 1.0f)  }
    }
};

static std::array<uint16_t, 36> g_Indicies = {
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

Cyrex::Graphics::Graphics() 
    :
    m_scissorRect(CD3DX12_RECT(0, 0, g_longMax, g_longMax))
{}

Cyrex::Graphics::~Graphics() = default;

void Cyrex::Graphics::Initialize(uint32_t width, uint32_t height) {
    //Check for DirectX Math suport
    if (!DirectX::XMVerifyCPUSupport()) {
        crxlog::err("No support for DirectX Math found");
        m_isIntialized = false;
        return;
    }

    m_device = Application::Get().GetDevice();
    m_clientWidth = width;
    m_clientHeight = height;
    
    CreateSwapChain(m_hWnd, m_clientWidth, m_clientHeight, m_bufferCount);
    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_bufferCount);

    m_rtvDescriptorHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    UpdateRenderTargetViews();

    LoadContent();
    Resize(width, height);
    m_isIntialized = true;
}

void Cyrex::Graphics::Update() noexcept {
    static uint64_t frameCounter = 0;

    m_timer.Tick();
    frameCounter++;

    if (m_timer.GetElapsedSeconds() > 1.0) {
        const auto fps = frameCounter / m_timer.GetElapsedSeconds();
        crxlog::normal(fps, " fps");
        frameCounter = 0;
        m_timer.ResetElapsedTime();
    }

    // Update the model matrix.
    float angle = static_cast<float>(m_timer.GetTotalSeconds() * 90);
    const dx::XMVECTOR rotationAxis = dx::XMVectorSet(0, 1, 1, 0);

    auto modelMatrix = dx::XMLoadFloat4x4(&m_modelMatrix);
    modelMatrix = dx::XMMatrixRotationAxis(rotationAxis, dx::XMConvertToRadians(angle));
    dx::XMStoreFloat4x4(&m_modelMatrix, modelMatrix);

    // Update the view matrix.
    const dx::XMVECTOR eyePosition = dx::XMVectorSet(0, 0, -10, 1);
    const dx::XMVECTOR focusPoint  = dx::XMVectorSet(0, 0, 0, 1);
    const dx::XMVECTOR upDirection = dx::XMVectorSet(0, 1, 0, 0);

    auto viewMatrix = dx::XMLoadFloat4x4(&m_viewMatrix);
    viewMatrix = dx::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
    dx::XMStoreFloat4x4(&m_viewMatrix, viewMatrix);

    // Update the projection matrix.
    float aspectratio = m_clientWidth / static_cast<float>(m_clientHeight);
    auto projectionMatrix = dx::XMLoadFloat4x4(&m_viewMatrix);

    projectionMatrix = dx::XMMatrixPerspectiveFovLH(
        dx::XMConvertToRadians(m_FoV), 
        aspectratio, 0.1f, 100.0f);
    dx::XMStoreFloat4x4(const_cast<dx::XMFLOAT4X4*>(&m_projectionMatrix), projectionMatrix);
}

void Cyrex::Graphics::Render() {
    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto commandList = commandQueue->GetCommandList();
    auto d3d12commandList = commandList->GetGraphicsCommandList();

    auto currentBackbufferIndex = GetCurrentBackBufferIndex();
    auto backbuffer = GetCurrentBackBuffer();
    auto rtv = GetCurrentRenderTargetView();
    auto dsv = m_dSVHeap->GetCPUDescriptorHandleForHeapStart();

    // Clear the render targets.
    {
        TransitionResource(d3d12commandList, backbuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        ClearRTV(d3d12commandList, rtv, DirectX::Colors::LightSteelBlue);
        ClearDepth(d3d12commandList, dsv);
    }

    commandList->SetPipelineState(m_pipelineState);
    commandList->SetGraphicsRootSignature(*m_rootSignature);

    commandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d12commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    d3d12commandList->IASetIndexBuffer(&m_indexBufferView);

    commandList->SetViewport(m_viewport);
    commandList->SetScissorRect(m_scissorRect);

    d3d12commandList->OMSetRenderTargets(1, &rtv, false, &dsv);

    // Update the MVP matrix
    auto modelMatrix = dx::XMLoadFloat4x4(&m_modelMatrix);
    auto viewMatrix  = dx::XMLoadFloat4x4(&m_viewMatrix);
    auto projMatrix  = dx::XMLoadFloat4x4(&m_projectionMatrix);

    dx::XMMATRIX mvpMatrix = dx::XMMatrixMultiply(modelMatrix, viewMatrix);
    mvpMatrix = dx::XMMatrixMultiply(mvpMatrix, projMatrix);
    commandList->SetGraphics32BitConstants(0, mvpMatrix);
   
    dx::XMStoreFloat4x4(&m_modelMatrix, modelMatrix);
    dx::XMStoreFloat4x4(&m_viewMatrix, viewMatrix);
    dx::XMStoreFloat4x4(&m_projectionMatrix, projMatrix);

    commandList->DrawIndexed(g_Indicies.size(), 1, 0, 0, 0);

    // Present
    {
        TransitionResource(d3d12commandList, backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_frameFenceValues.at(currentBackbufferIndex) = commandQueue->ExecuteCommandList(commandList);
        m_frameValues.at(currentBackbufferIndex) = Application::FrameCount();

        currentBackbufferIndex = Present();
        commandQueue->WaitForFenceValue(m_frameFenceValues.at(currentBackbufferIndex));
    }
}

void Cyrex::Graphics::Resize(uint32_t width, uint32_t height) {
    if (m_clientWidth != width != m_clientHeight != height) {
        m_clientWidth = std::max(1u, width);
        m_clientHeight = std::max(1u, height);

        Application::Get().Flush();

        for (int i = 0; i < m_bufferCount; ++i) {
            m_backBuffers[i].Reset();
        }

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        ThrowIfFailed(m_swapChain->GetDesc(&swapChainDesc));
        ThrowIfFailed(m_swapChain->ResizeBuffers(
            m_bufferCount,
            m_clientWidth,
            m_clientHeight,
            swapChainDesc.BufferDesc.Format,
            swapChainDesc.Flags));

        m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        UpdateRenderTargetViews();

        m_viewport = CD3DX12_VIEWPORT(
            0.0f, 0.0f, 
            static_cast<float>(m_clientWidth),
            static_cast<float>(m_clientHeight));

        ResizeDepthBuffer(m_clientWidth, m_clientHeight);
    }
}

void Cyrex::Graphics::UpdateBufferResource(
    wrl::ComPtr<ID3D12GraphicsCommandList2> commandList, 
    ID3D12Resource** ppDestinationResource, 
    ID3D12Resource** ppIntermediateResource, 
    size_t numElements, size_t elementSize, const void* bufferData, 
    D3D12_RESOURCE_FLAGS flags)
{
    size_t bufferSize = numElements * elementSize;

    ThrowIfFailed(m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(ppDestinationResource)));

    if (bufferData) {
        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(ppIntermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData      = bufferData;
        subresourceData.RowPitch   = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(),
            *ppDestinationResource,
            *ppIntermediateResource,
            0, 0, 1,
            &subresourceData);
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE Cyrex::Graphics::GetCurrentRenderTargetView() const {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        m_currentBackBufferIndex, 
        m_rtvDescriptorHeapSize);
}

wrl::ComPtr<ID3D12Resource> Cyrex::Graphics::GetCurrentBackBuffer() const {
    return m_backBuffers.at(m_currentBackBufferIndex);
}

uint32_t Cyrex::Graphics::GetCurrentBackBufferIndex() const {
    return m_currentBackBufferIndex;
}

uint32_t Cyrex::Graphics::Present() {
    uint32_t syncInterval = m_vsync ? 1 : 0;
    uint32_t presentFlags = Application::Get().IsTearingSupported() && !m_vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;

    ThrowIfFailed(m_swapChain->Present(syncInterval, presentFlags));
    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    return m_currentBackBufferIndex;
}

void Cyrex::Graphics::LoadContent() {
    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto commandList = commandQueue->GetCommandList();
    auto d3d12CommandList = commandList->GetGraphicsCommandList();

    // Upload vertex buffer data.
    wrl::ComPtr<ID3D12Resource> intermediateVertexBuffer;
    UpdateBufferResource(
        d3d12CommandList,
        &m_vertexBuffer,
        &intermediateVertexBuffer,
        g_Vertices.size(), 
        sizeof(VertexPosColor), 
        g_Vertices.data());

    // Create the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes    = sizeof(g_Vertices);
    m_vertexBufferView.StrideInBytes  = sizeof(VertexPosColor);

    // Upload index buffer data.
    wrl::ComPtr<ID3D12Resource> intermediateIndexBuffer;
    UpdateBufferResource(
        d3d12CommandList,
        &m_indexBuffer,
        &intermediateIndexBuffer,
        g_Indicies.size(),
        sizeof(WORD),
        g_Indicies.data());

    // Create index buffer view.
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format         = DXGI_FORMAT_R16_UINT;
    m_indexBufferView.SizeInBytes    = sizeof(g_Indicies);

    // Create the descriptor heap for the depth-stencil view.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dSVHeap)));

    // Load the vertex shader.
    wrl::ComPtr<ID3DBlob> vertexShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"D:\\Cyrex\\Cyrex\\x64\\Debug\\VertexShader.cso", &vertexShaderBlob));

    // Load the pixel shader.
    wrl::ComPtr<ID3DBlob> pixelShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"D:\\Cyrex\\Cyrex\\x64\\Debug\\PixelShader.cso", &pixelShaderBlob));

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},

        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, 
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS       |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS     |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS   |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};
    rootParameters[0].InitAsConstants(sizeof(dx::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    D3D12_ROOT_SIGNATURE_DESC1 rootSigDesc = {};
    rootSigDesc.NumParameters     = _countof(rootParameters);
    rootSigDesc.pParameters       = rootParameters;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers   = nullptr;
    rootSigDesc.Flags             = rootSignatureFlags;

    m_rootSignature = std::make_shared<RootSignature>(rootSigDesc, featureData.HighestVersion);

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipelineStateStream.pRootSignature        = m_rootSignature->GetRootSignature().Get();
    pipelineStateStream.InputLayout           = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.DSVFormat             = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats            = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(pipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(m_device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_pipelineState)));

    auto fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    ResizeDepthBuffer(m_clientWidth, m_clientHeight);
}

void Cyrex::Graphics::ToggleVsync() noexcept {
    m_vsync = !m_vsync;
}

void Cyrex::Graphics::OnMouseWheel(float delta) {
    m_FoV -= delta;
    m_FoV = std::clamp(m_FoV, 12.0f, 90.0f);

    crxlog::info("Fov: ", m_FoV);
}

void Cyrex::Graphics::CreateSwapChain(HWND hWnd, uint32_t width, uint32_t height, uint32_t bufferCount) {
    wrl::ComPtr<IDXGIFactory4> dxgiFactory4;
    uint32_t createFactoryFlags = 0;

#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width       = width;
    swapChainDesc.Height      = height;
    swapChainDesc.Format      = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo      = false;
    swapChainDesc.SampleDesc  = { 1,0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Scaling     = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags       = Application::Get().IsTearingSupported() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    auto commandQueue = Application::Get().GetCommandQueue()->GetD3D12CommandQueue();
    wrl::ComPtr<IDXGISwapChain1> swapchain1;

    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapchain1));

    // Disable the Alt + Enter fullscreen toggle feature.
    //Switching to fullscreen will be handled manually.
    ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    
    ThrowIfFailed(swapchain1.As(&m_swapChain));

    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void Cyrex::Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type = type;

    ThrowIfFailed(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvDescriptorHeap)));
}

void Cyrex::Graphics::UpdateRenderTargetViews() {
    const auto rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (uint8_t i = 0; i < m_bufferCount; ++i) {
        wrl::ComPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
        m_backBuffers.at(i) = backBuffer;

        rtvHandle.Offset(rtvDescriptorSize);
    }
}

void Cyrex::Graphics::ResizeDepthBuffer(uint32_t width, uint32_t height) {
    Application::Get().Flush();
    //don't allow width and height to be 0
    width  = std::max(1u, width);
    height = std::max(1u, height);

    D3D12_CLEAR_VALUE optimizedClearValue = {};
    optimizedClearValue.Format       = DXGI_FORMAT_D32_FLOAT;
    optimizedClearValue.DepthStencil = { 1.0f, 0 };

    ThrowIfFailed(m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
            1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &optimizedClearValue,
        IID_PPV_ARGS(&m_depthBuffer)
    ));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
    dsv.Format             = DXGI_FORMAT_D32_FLOAT;
    dsv.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsv.Texture2D.MipSlice = 0;
    dsv.Flags              = D3D12_DSV_FLAG_NONE;

    m_device->CreateDepthStencilView(
        m_depthBuffer.Get(), 
        &dsv,
        m_dSVHeap->GetCPUDescriptorHandleForHeapStart());
}

void Cyrex::Graphics::TransitionResource(
    wrl::ComPtr<ID3D12GraphicsCommandList2> commandList,
    wrl::ComPtr<ID3D12Resource> resource, 
    D3D12_RESOURCE_STATES beforeState, 
    D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        beforeState, 
        afterState);
    commandList->ResourceBarrier(1, &barrier);
}

void Cyrex::Graphics::ClearRTV(
    wrl::ComPtr<ID3D12GraphicsCommandList2> commandList, 
    D3D12_CPU_DESCRIPTOR_HANDLE rtv, 
    DirectX::XMVECTORF32 clearColor)
{
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void Cyrex::Graphics::ClearDepth(
    wrl::ComPtr<ID3D12GraphicsCommandList2> commandList, 
    D3D12_CPU_DESCRIPTOR_HANDLE dsv, 
    float depth)
{
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}
