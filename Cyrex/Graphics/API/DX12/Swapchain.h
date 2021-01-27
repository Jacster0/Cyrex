#pragma once
#include "RenderTarget.h"
#include <dxgi1_5.h>     
#include <wrl/client.h>  
#include <memory>
#include <array>

namespace Cyrex {
    class CommandQueue;
    class Device;
    class Texture;

    class SwapChain {
    public:
        static constexpr uint32_t BUFFER_COUNT = 3;
   
        bool IsTearingSupported() const noexcept { return m_tearingSupported; };
        void WaitForSwapChain();
        void Resize(uint32_t width, uint32_t height);
        uint32_t Present(const std::shared_ptr<Texture>& texture = nullptr);
        const RenderTarget& GetRenderTarget() const;
        DXGI_FORMAT GetRenderTargetFormat() { return m_renderTargetFormat; }
        Microsoft::WRL::ComPtr<IDXGISwapChain4> GetDXGISwapChain() const { return m_dxgiSwapChain; }

        void SetFullScreen(bool fullScreen);
        void ToggleFullScreen() { SetFullScreen(!m_fullscreen); }
        bool IsFullscreen() const noexcept { return m_fullscreen; }

        void ToggleVsync() { SetVsync(!m_vSync); }
        void SetVsync(bool vSync);
        bool GetVsync() const noexcept { return m_vSync; }
    protected:
        SwapChain(Device& device, HWND hWnd, DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R10G10B10A2_UNORM);
        virtual ~SwapChain();
        void UpdateRenderTargetViews(); 
    private:
        Device& m_device;
        CommandQueue& m_commandQueue;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_dxgiSwapChain;
        std::array <std::shared_ptr<Texture>, BUFFER_COUNT> m_backBufferTextures;
        mutable RenderTarget m_renderTarget;

        uint32_t m_currentBackBufferIndex;
        std::array<uint64_t, BUFFER_COUNT> m_fenceValues;

        HANDLE m_hFrameLatencyWaitableObject;
        HWND m_hWnd;
        uint32_t m_width;
        uint32_t m_height;
        DXGI_FORMAT m_renderTargetFormat;

        bool m_vSync;
        bool m_tearingSupported;
        bool m_fullscreen;
    };
}

