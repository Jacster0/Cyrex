#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <queue>
#include <memory>
#include <atomic>
#include <condition_variable>
#include "Core/ThreadSafeQueue.h"

namespace Cyrex {
    class CommandList;
    class Device;

    class CommandQueue {
    public:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const;
        std::shared_ptr<CommandList> GetCommandList();

       uint64_t ExecuteCommandList(std::shared_ptr<CommandList> commandList);
       uint64_t ExecuteCommandLists(const std::vector<std::shared_ptr<CommandList>>& commandLists);
        
        [[nodiscard]] uint64_t Signal();
        void WaitForFenceValue(uint64_t fenceValue);
        void Flush();
        bool IsFenceComplete(uint64_t fenceValue);
        void Wait(const CommandQueue& rhs);
    protected:
        friend class std::default_delete<CommandQueue>;

        CommandQueue(Device& device, D3D12_COMMAND_LIST_TYPE type);
        virtual ~CommandQueue();
    private:
        void ProccessInFlightCommandLists();
    private:
        using CommandListEntry = std::tuple<uint64_t, std::shared_ptr<CommandList>>;

        Device& m_device;
        D3D12_COMMAND_LIST_TYPE m_commandListType;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_d3d12Fence;
        std::atomic_uint64_t  m_fenceValue;

        ThreadSafeQueue<CommandListEntry> m_inFlightCommandLists;
        ThreadSafeQueue<std::shared_ptr<CommandList>> m_availableCommandLists;

        std::thread m_processInFlightCommandListsThread;
        std::atomic_bool m_processInFlightCommandListsBool = true;
        std::mutex m_processInFlightCommandListsThreadMutex;
        std::condition_variable m_processInFlightCommandListsThreadCV;
    };
}