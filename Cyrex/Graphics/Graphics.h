#pragma once
#include <memory>
#include <array>
#include "API/DX12/Common.h"

namespace Cyrex {
    struct VertexPosColor {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Color;
    };

    class RootSignature;
    class CommandQueue;
    class Graphics {
    public:
        Graphics();
        ~Graphics();
    public:
        void Initialize(uint32_t width, uint32_t height);
        void Update() const noexcept;
        void Render();
        void Resize(uint32_t width, uint32_t height);
        void LoadContent();
        void OnMouseWheel(float delta);
    public:
        void ToggleVsync() noexcept;
        void SetHwnd(HWND hWnd) noexcept { m_hWnd = hWnd; }
        bool IsInitialized() const noexcept { return m_isIntialized; }
    private:
        void CreateSwapChain(HWND hWnd, uint32_t width, uint32_t height, uint32_t bufferCount);
        void CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);

        void UpdateRenderTargetViews();
        void ResizeDepthBuffer(uint32_t width, uint32_t height);
        void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

        void ClearRTV(
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
            D3D12_CPU_DESCRIPTOR_HANDLE rtv, DirectX::XMVECTORF32 clearColor);
        void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
            D3D12_CPU_DESCRIPTOR_HANDLE dsv, float depth = 1.0f);
        void UpdateBufferResource(
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
            ID3D12Resource** ppDestinationResource,
            ID3D12Resource** ppIntermediateResource,
            size_t numElements, size_t elementSize, const void* bufferData,
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

        D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const;
        Microsoft::WRL::ComPtr<ID3D12Resource> GetCurrentBackBuffer() const;
        uint32_t GetCurrentBackBufferIndex() const;
        uint32_t Present();
    private:
        static constexpr uint8_t m_bufferCount = 3;
    private:
        Microsoft::WRL::ComPtr<ID3D12Device2> m_device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_d3d12commandQueue;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

        Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
        D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

        Microsoft::WRL::ComPtr<ID3D12Resource> m_depthBuffer;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dSVHeap;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

        D3D12_VIEWPORT m_viewport;
        D3D12_RECT m_scissorRect;

        float m_FoV = 30.0f;

        DirectX::XMFLOAT4X4 m_modelMatrix;
        DirectX::XMFLOAT4X4 m_viewMatrix;
        DirectX::XMFLOAT4X4 m_projectionMatrix;
    private:
        std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, m_bufferCount> m_backBuffers;
        std::array<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, m_bufferCount> m_commandAllocators;
    private:
        uint32_t m_rtvDescriptorHeapSize{0};
        uint32_t m_currentBackBufferIndex{0};

        std::array<uint64_t, m_bufferCount> m_frameFenceValues{};
        std::array<uint64_t, m_bufferCount> m_frameValues{};
        uint64_t m_fenceValue{ 0 };
        HANDLE m_fenceEvent;
      
        bool m_vsync = true;
        uint32_t m_clientWidth{};
        uint32_t m_clientHeight{};
        HWND m_hWnd;
        bool m_isIntialized = false;

        std::shared_ptr<RootSignature> m_rootSignature;
    };
}
