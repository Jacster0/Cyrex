#include "Swapchain.h"
#include "Device.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "RenderTarget.h"
#include "ResourceStateTracker.h"
#include "Texture.h"
#include "Adapter.h"
#include "DXException.h"

Cyrex::SwapChain::SwapChain(Device& device, HWND hWnd, DXGI_FORMAT renderTargetFormat)
    :
    m_device(device),
    m_commandQueue(device.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)),
    m_hWnd(hWnd),
    m_renderTargetFormat(renderTargetFormat)
{
    namespace wrl = Microsoft::WRL;

    assert(hWnd);

    auto d3d12CommandQueue = m_commandQueue.GetD3D12CommandQueue();

    auto adapter           = m_device.GetAdapter();
    auto dxgiAdapter       = adapter->GetDXGIAdapter();

    wrl::ComPtr<IDXGIFactory>  dxgiFactory;
    wrl::ComPtr<IDXGIFactory5> dxgiFactory5;

    ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));
    ThrowIfFailed(dxgiFactory.As(&dxgiFactory5));

    BOOL allowTearing = FALSE;
    if (SUCCEEDED(
        dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(BOOL))))
    {
        m_tearingSupported = (allowTearing == TRUE);
    }

    RECT windowRect;
    GetClientRect(hWnd, &windowRect);

    m_width  = windowRect.right  - windowRect.left;
    m_height = windowRect.bottom - windowRect.top;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width       = m_width;
    swapChainDesc.Height      = m_height;
    swapChainDesc.Format      = m_renderTargetFormat;
    swapChainDesc.Stereo      = FALSE;
    swapChainDesc.SampleDesc  = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = BUFFER_COUNT;
    swapChainDesc.Scaling     = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags       = m_tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    swapChainDesc.Flags      |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    wrl::ComPtr<IDXGISwapChain1> dxgiSwapChain1;

    ThrowIfFailed(dxgiFactory5->CreateSwapChainForHwnd(
        d3d12CommandQueue.Get(), 
        m_hWnd, 
        &swapChainDesc, 
        nullptr,
        nullptr, 
        &dxgiSwapChain1));

    ThrowIfFailed(dxgiSwapChain1.As(&m_dxgiSwapChain));
    ThrowIfFailed(dxgiFactory5->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));

    m_currentBackBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();
    m_dxgiSwapChain->SetMaximumFrameLatency(BUFFER_COUNT - 1);
    m_hFrameLatencyWaitableObject = m_dxgiSwapChain->GetFrameLatencyWaitableObject();

    UpdateRenderTargetViews();
}

Cyrex::SwapChain::~SwapChain()
{
}

void Cyrex::SwapChain::WaitForSwapChain() {
    auto result = WaitForSingleObjectEx(m_hFrameLatencyWaitableObject, 1000, true);
}

void Cyrex::SwapChain::Resize(uint32_t width, uint32_t height) {
    if (m_width != width || m_height != height) {
        m_width  = std::max(1u, width);
        m_height = std::max(1u, height);

        m_device.Flush();

        // Release all references to back buffer textures.
        m_renderTarget.Reset();

        for (auto i = 0; i < BUFFER_COUNT; i++) {
            //ResourceStateTracker::RemoveGlobalResourceState(m_backBufferTextures[i]->GetD3D12Resource().Get(), true);
            m_backBufferTextures[i].reset();
        }

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        ThrowIfFailed(m_dxgiSwapChain->GetDesc(&swapChainDesc));
        ThrowIfFailed(m_dxgiSwapChain->ResizeBuffers(
            BUFFER_COUNT,
            m_width, 
            m_height, 
            swapChainDesc.BufferDesc.Format,
            swapChainDesc.Flags));

        m_currentBackBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();
        UpdateRenderTargetViews();
    }
}

uint32_t Cyrex::SwapChain::Present(const std::shared_ptr<Texture>& texture) {
    const auto commandList = m_commandQueue.GetCommandList();
    const auto backBuffer  = m_backBufferTextures.at(m_currentBackBufferIndex);

    if (texture) {
        if (texture->GetD3D12ResourceDesc().SampleDesc.Count > 1) {
            commandList->ResolveSubresource(backBuffer, texture);
        }
        else {
            commandList->CopyResource(backBuffer, texture);
        }
    }
    commandList->TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    m_commandQueue.ExecuteCommandList(commandList);

    uint32_t syncInterval = static_cast<uint32_t>(m_vSync);
    uint32_t presentFlags = (m_tearingSupported && !m_fullscreen && m_vSync == VSync::Off) ? DXGI_PRESENT_ALLOW_TEARING : 0;

    ThrowIfFailed(m_dxgiSwapChain->Present(syncInterval, presentFlags));

    m_fenceValues.at(m_currentBackBufferIndex) = m_commandQueue.Signal();

    m_currentBackBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();
    m_device.ReleaseStaleDescriptors();

    return m_currentBackBufferIndex;
}

const Cyrex::RenderTarget& Cyrex::SwapChain::GetRenderTarget() const {
    m_renderTarget.AttachTexture(AttachmentPoint::Color0, m_backBufferTextures[m_currentBackBufferIndex]);

    return m_renderTarget;
} 

void Cyrex::SwapChain::UpdateRenderTargetViews() {
    namespace wrl = Microsoft::WRL;

    for (uint32_t i = 0; i < BUFFER_COUNT; i++) {
        wrl::ComPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(m_dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        ResourceStateTracker::AddGlobalResourceState(backBuffer.Get(), D3D12_RESOURCE_STATE_COMMON);

        m_backBufferTextures[i] = m_device.CreateTexture(backBuffer);
        m_backBufferTextures[i]->SetName(L"Backbuffer[" + std::to_wstring(i) + L"]");
    }
}

void Cyrex::SwapChain::SetFullScreen(bool fullScreen) {
    if (m_fullscreen != fullScreen) {
        m_fullscreen = fullScreen;
    }
}

void Cyrex::SwapChain::SetVsync(VSync vSync) {
    m_vSync = vSync;
}
