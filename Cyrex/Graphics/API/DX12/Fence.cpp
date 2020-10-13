#include "Fence.h"
#include "DXException.h"

namespace wrl = Microsoft::WRL;

wrl::ComPtr<ID3D12Fence> Cyrex::Fence::Create(wrl::ComPtr<ID3D12Device2> device) {
    wrl::ComPtr<ID3D12Fence> fence;
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return fence;
}

HANDLE Cyrex::Fence::CreateEventHandle() {  
    auto fenceEvent = CreateEvent(nullptr, false, false, nullptr);
    assert(fenceEvent && "Failed to create fence event.");

    return fenceEvent;
}

uint64_t Cyrex::Fence::Signal(
    ID3D12Fence* fence,
    wrl::ComPtr<ID3D12CommandQueue> commandQueue,
    uint64_t& fenceValue)
{
    uint64_t fenceValueForSignal = ++fenceValue;
    ThrowIfFailed(commandQueue->Signal(fence, fenceValueForSignal));

    return fenceValueForSignal;
}

void Cyrex::Fence::WaitForFenceValue( 
    ID3D12Fence* fence,
    HANDLE fenceEvent,
    uint64_t fenceValue,
    std::chrono::milliseconds duration)
{
    if (!IsFenceComplete(fence, fenceValue)) {
        ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
        WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
    }
}

void Cyrex::Fence::Flush(
    ID3D12Fence* fence,
    HANDLE fenceEvent,
    wrl::ComPtr<ID3D12CommandQueue> commandQueue, 
    uint64_t& fenceValue)
{
    uint64_t fenceValueForSignal = Signal(fence,commandQueue,fenceValue);
    Fence::WaitForFenceValue(fence, fenceEvent,fenceValueForSignal);
}

bool Cyrex::Fence::IsFenceComplete(ID3D12Fence* fence, uint64_t fenceValue) {
    return fence->GetCompletedValue() >= fenceValue;
}