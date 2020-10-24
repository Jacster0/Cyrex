#include "CommandQueue.h"
#include "DXException.h"
#include "CommandList.h"
#include "ResourceStateTracker.h"
#include <cassert>

namespace wrl = Microsoft::WRL;

Cyrex::CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type, ID3D12Device2* device)
    :
    m_fenceValue(0),
    m_commandListType(type),
    m_device(device)
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type     = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
    ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence)));

    m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
    assert(m_fenceEvent && "Failed to create fence event handle.");
}

wrl::ComPtr<ID3D12CommandQueue> Cyrex::CommandQueue::GetD3D12CommandQueue() const
{
    return m_d3d12CommandQueue;
}

std::shared_ptr<Cyrex::CommandList> Cyrex::CommandQueue::GetCommandList() {
    std::shared_ptr<CommandList> commandList;

    if (!m_commandListQueue.empty() && IsFenceComplete(m_commandListQueue.front().FenceValue)) {
        commandList = m_commandListQueue.front().CommandList;
        m_commandListQueue.pop();

        commandList->Reset();

    }
    else {
        commandList = std::make_shared<CommandList>(m_commandListType,m_device);
    }
    return commandList;
}

uint64_t Cyrex::CommandQueue::ExecuteCommandList(std::shared_ptr<CommandList> commandList) {
    ResourceStateTracker::Lock();
    auto pendingCommandList = GetCommandList();

    commandList->Close(*pendingCommandList);
    pendingCommandList->Close();

    ID3D12CommandList* const ppCommandLists[] = {
        pendingCommandList->GetGraphicsCommandList().Get(),
        commandList->GetGraphicsCommandList().Get()
    };

    m_d3d12CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    auto fenceValue = Signal();

    ResourceStateTracker::Unlock();

    m_commandListQueue.emplace(CommandListEntry{ fenceValue, pendingCommandList });
    m_commandListQueue.emplace(CommandListEntry{ fenceValue, commandList });

    return fenceValue;
}

uint64_t Cyrex::CommandQueue::Signal() {
    uint64_t fenceValue = ++m_fenceValue;
    m_d3d12CommandQueue->Signal(m_d3d12Fence.Get(), m_fenceValue);
    return fenceValue;
}

void Cyrex::CommandQueue::WaitForFenceValue(uint64_t fenceValue) {
    if (!IsFenceComplete(fenceValue)) {
        m_d3d12Fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, std::numeric_limits<unsigned long>::max());
    }
}

void Cyrex::CommandQueue::Flush(uint64_t fenceValue) {
    WaitForFenceValue(Signal());
}

bool Cyrex::CommandQueue::IsFenceComplete(uint64_t fenceValue) {
    return m_d3d12Fence->GetCompletedValue() >= fenceValue;
}