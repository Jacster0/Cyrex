#pragma once
#include "Buffer.h"
#include "DescriptorAllocation.h"
#include "d3dx12.h"

namespace Cyrex {
    class Device;

    class ByteAddressBuffer : public Buffer {
    public:
        size_t GetBufferSize() const noexcept {
            return m_bufferSize;
        }
    protected:
        ByteAddressBuffer(Device& device, const D3D12_RESOURCE_DESC& resDesc);
        ByteAddressBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource);
        virtual ~ByteAddressBuffer() = default;
    private:
        size_t m_bufferSize;
    };
}