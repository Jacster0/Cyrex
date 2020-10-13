#include <chrono>
#include "Graphics.h"
#include "API/DX12/DXException.h"
#include "Core/Logger.h"
#include "API/DX12/Fence.h"

namespace wrl = Microsoft::WRL;

Cyrex::Graphics::Graphics() = default;

Cyrex::Graphics::~Graphics() = default;

void Cyrex::Graphics::Initialize() {
    EnableDebugLayer();
    wrl::ComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter(false);
    CreateDevice(dxgiAdapter4);
    CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    CreateSwapChain(m_hWnd, m_clientWidth, m_clientHeight, m_numFrames);

    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
    CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_numFrames);

    m_rtvDescriptorHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    UpdateRenderTargetViews();

    for (uint8_t i = 0; i < m_numFrames; ++i) {
        m_commandAllocators.at(i) = CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
    }
    CreateCommandList(m_commandAllocators.at(m_currentBackBufferIndex), D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_fence = Fence::Create(m_device);
    m_fenceEvent = Fence::CreateEventHandle();

    m_isIntialized = true;
}

void Cyrex::Graphics::Update() const noexcept {
    static uint64_t frameCounter = 0;
    static double elapsedSeconds = 0.0;
    static std::chrono::high_resolution_clock clock;
    static auto t0 = clock.now();

    frameCounter++;
    auto t1 = clock.now();
    auto deltaTime = t1 - t0;
    t0 = t1;

    elapsedSeconds += deltaTime.count() * 1e-9;

    if (elapsedSeconds > 1.0) {
        const auto fps = frameCounter / elapsedSeconds;
        crxlog::normal(fps, " fps");
        frameCounter = 0;
        elapsedSeconds = 0.0;
    }
}

void Cyrex::Graphics::Render() {
    auto commandAllocator = m_commandAllocators[m_currentBackBufferIndex];
    auto backBuffer = m_backBuffers[m_currentBackBufferIndex];

    commandAllocator->Reset();
    m_commandList->Reset(commandAllocator.Get(), nullptr);
    
    // Clear the render target
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        m_commandList->ResourceBarrier(1, &barrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
            m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            m_currentBackBufferIndex,
            m_rtvDescriptorHeapSize);

        m_commandList->ClearRenderTargetView(rtv, DirectX::Colors::LightSteelBlue, 0, nullptr);
    }

    //present
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &barrier);

        ThrowIfFailed(m_commandList->Close());

        ID3D12CommandList* const commandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

        uint32_t syncInterval = m_vsync;
        uint32_t presentFlags = CheckTearingSupport() && !m_vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;
        ThrowIfFailed(m_swapChain->Present(syncInterval, presentFlags));

        m_frameFenceValues.at(m_currentBackBufferIndex) = Fence::Signal(m_fence.Get(), m_commandQueue, m_fenceValue);

        m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
        Fence::WaitForFenceValue(m_fence.Get(), m_fenceEvent, m_frameFenceValues.at(m_currentBackBufferIndex));
    }
   
}

void Cyrex::Graphics::Resize(uint32_t width, uint32_t height) {
    if (m_clientWidth != width != m_clientHeight != height) {
        //Don't allow 0 size swap chain back buffers
        m_clientWidth = std::max(1u, width);
        m_clientHeight = std::max(1u, height);

        Fence::Flush(m_fence.Get(), m_fenceEvent, m_commandQueue, m_fenceValue);

        for (uint8_t i = 0; i < m_numFrames; ++i) {
            m_backBuffers.at(i).Reset();
            m_frameFenceValues.at(i) = m_frameFenceValues[m_currentBackBufferIndex];
        }

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        ThrowIfFailed(m_swapChain->GetDesc(&swapChainDesc));
        ThrowIfFailed(m_swapChain->ResizeBuffers(
            m_numFrames, 
            m_clientWidth, 
            m_clientHeight,
            swapChainDesc.BufferDesc.Format, 
            swapChainDesc.Flags));

        m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
        UpdateRenderTargetViews();
    }
}

void Cyrex::Graphics::ToggleVsync() {
    m_vsync = !m_vsync;
}

void Cyrex::Graphics::EnableDebugLayer() const {
#if defined(_DEBUG)
    wrl::ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif
}

wrl::ComPtr<IDXGIAdapter4> Cyrex::Graphics::GetAdapter(bool useWarp) {
    wrl::ComPtr<IDXGIFactory4> dxgiFactory;
    uint32_t createFactoryFlags = 0;

#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    wrl::ComPtr<IDXGIAdapter1> dxgiAdapter1;
    wrl::ComPtr<IDXGIAdapter4> dxgiAdapter4;

    if (useWarp) {
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
    }
    else {
        size_t maxDedicatedVideoMemory = 0;

        for (uint32_t i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(
                    dxgiAdapter1.Get(),
                    D3D_FEATURE_LEVEL_11_0,
                    __uuidof(ID3D12Device),
                    nullptr)) &&
                dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
            {
                maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
            }
        }
    }

    return dxgiAdapter4;
}

void Cyrex::Graphics::CreateDevice(wrl::ComPtr<IDXGIAdapter4> adapter) {
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));

    //enable debug messages in debug mode
#if defined(_DEBUG)
    wrl::ComPtr<ID3D12InfoQueue> p_infoQueue;

    if (SUCCEEDED(m_device.As(&p_infoQueue))) {
        p_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        p_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        p_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        std::array severities = { D3D12_MESSAGE_SEVERITY_INFO };
        std::array denyIds = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
        };
       
        D3D12_INFO_QUEUE_FILTER newFilter = {};
        newFilter.DenyList.NumSeverities  = severities.size();
        newFilter.DenyList.pSeverityList  = severities.data();
        newFilter.DenyList.NumIDs         = denyIds.size();
        newFilter.DenyList.pIDList        = denyIds.data();

        ThrowIfFailed(p_infoQueue->PushStorageFilter(&newFilter));
    }
#endif
}

void Cyrex::Graphics::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type) {
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type     = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue)));
}

bool Cyrex::Graphics::CheckTearingSupport() const {
    bool allowTearing = true;

    wrl::ComPtr<IDXGIFactory4> factory4;

    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4)))) {
        wrl::ComPtr<IDXGIFactory5> factory5;

        if (SUCCEEDED(factory4.As(&factory5))) {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing,
                sizeof(allowTearing))))
            {
                allowTearing = false;
            }
        }
    }

    return allowTearing;
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
    swapChainDesc.Flags       = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    wrl::ComPtr<IDXGISwapChain1> swapchain1;
    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        m_commandQueue.Get(),
        hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapchain1));

    // Disable the Alt + Enter fullscreen toggle feature.Switching to fullscreen
    // will be handled manually.
    ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    
    ThrowIfFailed(swapchain1.As(&m_swapChain));
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

    for (uint8_t i = 0; i < m_numFrames; ++i) {
        wrl::ComPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
        m_backBuffers.at(i) = backBuffer;

        rtvHandle.Offset(rtvDescriptorSize);
    }
}

wrl::ComPtr<ID3D12CommandAllocator> Cyrex::Graphics::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type) {
    wrl::ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(m_device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

    return commandAllocator;
}

void Cyrex::Graphics::CreateCommandList(wrl::ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type) {
    ThrowIfFailed(m_device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
    ThrowIfFailed(m_commandList->Close());
}
