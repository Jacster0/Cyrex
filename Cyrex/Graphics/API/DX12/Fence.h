#pragma once
#include "Platform/Windows/CrxWindow.h"
#include "Common.h"
#include <wrl.h>
#include <chrono>

namespace Cyrex {
    class Fence {
    public:
        static Microsoft::WRL::ComPtr<ID3D12Fence> Create(Microsoft::WRL::ComPtr<ID3D12Device2> device);
        static HANDLE CreateEventHandle();

        static uint64_t Signal(
            ID3D12Fence* fence,
            Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue, 
            uint64_t& fenceValue);

        static void WaitForFenceValue(
            ID3D12Fence* fence,
            HANDLE fenceEvent,
            uint64_t fenceValue, 
            std::chrono::milliseconds duration = std::chrono::milliseconds::max());

        static void Flush(
            ID3D12Fence* fence,
            HANDLE fenceEvent,
            Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue, 
            uint64_t& fenceValue);

        static bool IsFenceComplete(ID3D12Fence* fence, uint64_t fenceValue);
    private:
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        HANDLE m_fenceEvent;
    };
}
