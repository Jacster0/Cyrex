#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <queue>
#include <memory>

namespace Cyrex {
    class CommandList;
    class CommandQueue {
    public:
        CommandQueue(D3D12_COMMAND_LIST_TYPE type, ID3D12Device2* device);
    public:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const;
        std::shared_ptr<CommandList> GetCommandList();
        [[nodiscard]] uint64_t ExecuteCommandList(std::shared_ptr<CommandList> commandList);
    public:
        [[nodiscard]] uint64_t Signal();
        void WaitForFenceValue(uint64_t fenceValue);
        void Flush(uint64_t fenceValue);
        bool IsFenceComplete(uint64_t fenceValue);
    private:
        struct CommandListEntry {
            uint64_t FenceValue;
            std::shared_ptr<CommandList> CommandList;
        };

        using CommandListQueue = std::queue<CommandListEntry>;

        D3D12_COMMAND_LIST_TYPE m_commandListType;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_d3d12Fence;
        ID3D12Device2* m_device;
        HANDLE m_fenceEvent;
        uint64_t m_fenceValue;

        CommandListQueue m_commandListQueue;
    };
}