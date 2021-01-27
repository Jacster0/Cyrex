#pragma once

#include "Buffer.h"
#include "ByteAddressBuffer.h"

namespace Cyrex {
    class StructuredBuffer : public Buffer {
    public:
        virtual size_t GetNumElements() const noexcept { return m_numElements; }
        virtual size_t GetElementSize() const noexcept { return m_elementSize; }
        std::shared_ptr<ByteAddressBuffer> GetCounterBuffer() const noexcept { return m_counterBuffer; }
    protected:
        StructuredBuffer(
            Device& device, 
            size_t numElements,
            size_t elementSize);

        StructuredBuffer(
            Device& device, 
            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            size_t numElements, size_t elementSize);

        virtual ~StructuredBuffer() = default;
    private:
        size_t m_numElements;
        size_t m_elementSize;

        std::shared_ptr<ByteAddressBuffer> m_counterBuffer;
    };
}