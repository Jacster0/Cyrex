#include "Core/Utils/ThreadUtils.h"
#include "CommandQueue.h"
#include "DXException.h"
#include "CommandList.h"
#include "ResourceStateTracker.h"
#include "Device.h"
#include <cassert>
#include <string>

namespace wrl = Microsoft::WRL;

namespace Context {
    using namespace Cyrex;

    class MakeCommandList final : public CommandList {
    public:
        MakeCommandList(Device& device, D3D12_COMMAND_LIST_TYPE type)
            :
            CommandList(device, type)
        {}
        ~MakeCommandList() {}
    };
}
Cyrex::CommandQueue::CommandQueue(Device& device, D3D12_COMMAND_LIST_TYPE type)
    :
    m_device(device),
    m_fenceValue(0),
    m_commandListType(type)
{
    using namespace std::string_literals;

    const auto d3d12Device = m_device.GetD3D12Device();

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type     = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(d3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
    ThrowIfFailed(d3d12Device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence)));

    auto threadname = "ProccessInFlightCommandLists "s;

    switch (type) {
    case D3D12_COMMAND_LIST_TYPE_COPY :
        m_d3d12CommandQueue->SetName(L"Copy Command Queue");
        threadname.append("(Copy)");
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        m_d3d12CommandQueue->SetName(L"Compute Command Queue");
        threadname.append("(Compute)");
        break;
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        m_d3d12CommandQueue->SetName(L"Direct Command Queue");
        threadname.append("(Direct)");
        break;
    default:
        break;
    }

    m_processInFlightCommandListsThread = std::thread(&CommandQueue::ProccessInFlightCommandLists, this);
    ThreadUtils::SetThreadName(m_processInFlightCommandListsThread, threadname.c_str());
}

wrl::ComPtr<ID3D12CommandQueue> Cyrex::CommandQueue::GetD3D12CommandQueue() const
{
    return m_d3d12CommandQueue;
}

std::shared_ptr<Cyrex::CommandList> Cyrex::CommandQueue::GetCommandList() {
    std::shared_ptr<CommandList> commandList;

    if (!m_availableCommandLists.Empty()) {
        m_availableCommandLists.TryPop(commandList);
    }
    else {
        commandList = std::make_shared<Context::MakeCommandList>(m_device, m_commandListType);
    }

    return commandList;
}

uint64_t Cyrex::CommandQueue::ExecuteCommandList(std::shared_ptr<CommandList> commandList) {
    return ExecuteCommandLists(std::vector<std::shared_ptr<CommandList>>({ commandList }));
}

uint64_t Cyrex::CommandQueue::ExecuteCommandLists(const std::vector<std::shared_ptr<CommandList>>& commandLists) {
    ResourceStateTracker::Lock();

    auto capacity = commandLists.size() * 2; //2x since each command list will have a pending command list

    std::vector<std::shared_ptr<CommandList>> toBeQueued;
    toBeQueued.reserve(capacity);

    std::vector<std::shared_ptr<CommandList>> generateMipsCommandLists;
    generateMipsCommandLists.reserve(commandLists.size());

    std::vector<ID3D12CommandList*> d3d12CommandLists;
    d3d12CommandLists.reserve(capacity); 

    for (const auto commandList : commandLists) {
        const auto pendingCommandList = GetCommandList();

        bool hasPendingBarriers = commandList->Close(pendingCommandList);
        pendingCommandList->Close();

        // If there are no pending barriers on the pending command list, there is no reason to
        // execute an empty command list on the command queue.
        if (hasPendingBarriers) {
            d3d12CommandLists.push_back(pendingCommandList->GetD3D12CommandList().Get());
        }
        d3d12CommandLists.push_back(commandList->GetD3D12CommandList().Get());

        toBeQueued.push_back(pendingCommandList);
        toBeQueued.push_back(commandList);

        auto generateMipsCommandList = commandList->GetGenerateMipsCommandList();

        if (generateMipsCommandList) {
            generateMipsCommandLists.push_back(generateMipsCommandList);
        }
    }

    auto numCommandLists = d3d12CommandLists.size();
    m_d3d12CommandQueue->ExecuteCommandLists(numCommandLists, d3d12CommandLists.data());
    uint64_t fenceValue = Signal();

    ResourceStateTracker::Unlock();

    for (const auto commandList : toBeQueued) {
        m_inFlightCommandLists.Push({ fenceValue,commandList });
    }

    // If there are any command lists that generate mips then execute those
    // after the initial resource command lists have finished.
    if (!generateMipsCommandLists.empty()) {
        auto& computeQueue = m_device.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);

        computeQueue.Wait(*this);
        computeQueue.ExecuteCommandLists(generateMipsCommandLists);
    }
    return fenceValue;
}

uint64_t Cyrex::CommandQueue::Signal() {
    uint64_t fenceValue = ++m_fenceValue;
    m_d3d12CommandQueue->Signal(m_d3d12Fence.Get(), m_fenceValue);
    return fenceValue;
}

void Cyrex::CommandQueue::WaitForFenceValue(uint64_t fenceValue) {
    if (!IsFenceComplete(fenceValue)) {
        auto event = CreateEvent(nullptr, false, false, nullptr);

        if (event) {
            m_d3d12Fence->SetEventOnCompletion(fenceValue, event);

            WaitForSingleObject(event, std::numeric_limits<DWORD>::max());
            CloseHandle(event);
        }
    }
}

void Cyrex::CommandQueue::Flush() {
    std::unique_lock<std::mutex> lock(m_processInFlightCommandListsThreadMutex);
    m_processInFlightCommandListsThreadCV.wait(lock, [this] { return m_inFlightCommandLists.Empty(); });

    WaitForFenceValue(m_fenceValue);
}

bool Cyrex::CommandQueue::IsFenceComplete(uint64_t fenceValue) {
    return m_d3d12Fence->GetCompletedValue() >= fenceValue;
}

void Cyrex::CommandQueue::Wait(const CommandQueue& rhs) {
    m_d3d12CommandQueue->Wait(rhs.m_d3d12Fence.Get(), rhs.m_fenceValue);
}

Cyrex::CommandQueue::~CommandQueue() {
    m_processInFlightCommandListsBool = false;
    m_processInFlightCommandListsThread.join();
}

void Cyrex::CommandQueue::ProccessInFlightCommandLists() {
    using namespace std::chrono_literals;

    std::unique_lock<std::mutex> lock(m_processInFlightCommandListsThreadMutex, std::defer_lock);

    while (m_processInFlightCommandListsBool) {
        CommandListEntry cmdListEntry;

        lock.lock();

        while (m_inFlightCommandLists.TryPop(cmdListEntry)) {
            auto [fenceValue, commandList] = cmdListEntry;

            WaitForFenceValue(fenceValue);
            commandList->Reset();

            m_availableCommandLists.Push(commandList);
        }

        lock.unlock();
        m_processInFlightCommandListsThreadCV.notify_one();

        std::this_thread::yield();
    }
}
