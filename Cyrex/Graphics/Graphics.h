#pragma once
#include <memory>
#include <array>
#include "API/DX12/Common.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace Cyrex {
    class Graphics {
    public:
        Graphics();
        ~Graphics();
    public:
        void Initialize();
        void Update() const noexcept;
        void Render();
        void Resize(uint32_t width, uint32_t height);
    public:
        void ToggleVsync();
        void SetHwnd(HWND hWnd) { m_hWnd = hWnd; }
        bool IsInitialized() const { return m_isIntialized; }
    private:
        void EnableDebugLayer() const;
        Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);
        void CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
        void CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type);
        bool CheckTearingSupport() const;
        void CreateSwapChain(HWND hWnd, uint32_t width, uint32_t height, uint32_t bufferCount);
        void CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
        void UpdateRenderTargetViews();
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type);
        void CreateCommandList(
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator,
            D3D12_COMMAND_LIST_TYPE type);
    private:
        static constexpr uint8_t m_numFrames = 3;
    private:
        Microsoft::WRL::ComPtr<ID3D12Device2> m_device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;

        std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, m_numFrames> m_backBuffers;
        std::array<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, m_numFrames> m_commandAllocators;
    private:
        uint32_t m_rtvDescriptorHeapSize{0};
        uint32_t m_currentBackBufferIndex{0};

        std::array<uint64_t, m_numFrames> m_frameFenceValues{};
        uint64_t m_fenceValue{ 0 };
        HANDLE m_fenceEvent;
      
        bool m_vsync = true;
        uint32_t m_clientWidth{};
        uint32_t m_clientHeight{};
        HWND m_hWnd;
        bool m_isIntialized = false;
    };
}
