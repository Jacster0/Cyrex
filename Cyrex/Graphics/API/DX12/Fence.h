#pragma once
#include "Platform/Windows/CrxWindow.h"
#include "Common.h"
#include <wrl.h>
#include <chrono>

namespace Cyrex {
    class Fence {
    public:
        Microsoft::WRL::ComPtr<ID3D12Fence> Create(Microsoft::WRL::ComPtr<ID3D12Device2> device);
        HANDLE CreateEventHandle();
        uint64_t Signal(
            Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue, 
            uint64_t& fenceValue) const;
        void WaitForFenceValue( 
            uint64_t fenceValue, 
            std::chrono::milliseconds duration) const;
        void Flush(
            Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue, 
            uint64_t& fenceValue) const;
    private:
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        HANDLE m_fenceEvent;
    };
}
