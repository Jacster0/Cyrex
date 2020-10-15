#include "CommandQueue.h"
#include "DXException.h"
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

wrl::ComPtr<ID3D12GraphicsCommandList2> Cyrex::CommandQueue::GetCommandList() {
    wrl::ComPtr<ID3D12CommandAllocator> commandAllocator;
    wrl::ComPtr<ID3D12GraphicsCommandList2> commandList;

    if (!m_commandAllocatorQueue.empty() && IsFenceComplete(m_commandAllocatorQueue.front().fenceValue))
    {
        commandAllocator = m_commandAllocatorQueue.front().commandAllocator;
        m_commandAllocatorQueue.pop();

        ThrowIfFailed(commandAllocator->Reset());
    }
    else {
        commandAllocator = CreateCommandAllocator();
    }

    if (!m_commandListQueue.empty()) {
        commandList = m_commandListQueue.front();
        m_commandListQueue.pop();

        ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
    }
    else {
        commandList = CreateCommandList(commandAllocator);
    }

    ThrowIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

    return commandList;
}

uint64_t Cyrex::CommandQueue::ExecuteCommandList(wrl::ComPtr<ID3D12GraphicsCommandList2> commandList) {
    commandList->Close();

    ID3D12CommandAllocator* commandAllocator;
    uint32_t dataSize = sizeof(commandAllocator);
    ThrowIfFailed(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

    ID3D12CommandList* const ppCommandList[] = { commandList.Get() };

    m_d3d12CommandQueue->ExecuteCommandLists(1, ppCommandList);
    auto fenceValue = Signal();

    m_commandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
    m_commandListQueue.push(commandList);

    commandAllocator->Release();

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

wrl::ComPtr<ID3D12CommandAllocator> Cyrex::CommandQueue::CreateCommandAllocator() const {
    wrl::ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(m_device->CreateCommandAllocator(m_commandListType, IID_PPV_ARGS(&commandAllocator)));

    return commandAllocator;
}

wrl::ComPtr<ID3D12GraphicsCommandList2> Cyrex::CommandQueue::CreateCommandList(
    wrl::ComPtr<ID3D12CommandAllocator> allocator) const
{
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList;
    ThrowIfFailed(m_device->CreateCommandList(
        0,
        m_commandListType,
        allocator.Get(),
        nullptr,
        IID_PPV_ARGS(&commandList)
    ));

    return commandList;
}
