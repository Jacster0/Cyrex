#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <queue>

namespace Cyrex {
    class CommandQueue {
    public:
        CommandQueue(D3D12_COMMAND_LIST_TYPE type, ID3D12Device2* device);
    public:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();
        [[nodiscard]] uint64_t ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);
    public:
        [[nodiscard]] uint64_t Signal();
        void WaitForFenceValue(uint64_t fenceValue);
        void Flush(uint64_t fenceValue);
        bool IsFenceComplete(uint64_t fenceValue);
    public:
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator() const;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator) const;
    private:
        struct CommandAllocatorEntry {
            uint64_t fenceValue;
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
        };
        using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
        using CommandListQueue = std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>>;

        D3D12_COMMAND_LIST_TYPE m_commandListType;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_d3d12Fence;
        ID3D12Device2* m_device;
        HANDLE m_fenceEvent;
        uint64_t m_fenceValue;

        CommandAllocatorQueue m_commandAllocatorQueue;
        CommandListQueue m_commandListQueue;
    };
}