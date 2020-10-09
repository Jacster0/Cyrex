#include "Fence.h"
#include "DXException.h"

namespace wrl = Microsoft::WRL;

wrl::ComPtr<ID3D12Fence> Cyrex::Fence::Create(wrl::ComPtr<ID3D12Device2> device) {
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
    return m_fence;
}

HANDLE Cyrex::Fence::CreateEventHandle() {  
    m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
    assert(m_fenceEvent && "Failed to create fence event.");

    return m_fenceEvent;
}

uint64_t Cyrex::Fence::Signal(
    wrl::ComPtr<ID3D12CommandQueue> commandQueue,
    uint64_t& fenceValue) const
{
    uint64_t fenceValueForSignal = ++fenceValue;
    ThrowIfFailed(commandQueue->Signal(m_fence.Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

void Cyrex::Fence::WaitForFenceValue( 
    uint64_t fenceValue,
    std::chrono::milliseconds duration) const
{
    if (m_fence->GetCompletedValue() < fenceValue) {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, static_cast<DWORD>(duration.count()));
    }
}

void Cyrex::Fence::Flush(
    wrl::ComPtr<ID3D12CommandQueue> commandQueue, 
    uint64_t& fenceValue) const
{
    uint64_t fenceValueForSignal = Signal(commandQueue,fenceValue);
    WaitForFenceValue(fenceValueForSignal);
}
