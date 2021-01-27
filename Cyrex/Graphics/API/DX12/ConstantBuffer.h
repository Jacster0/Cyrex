#pragma once

#include "Buffer.h"
#include <d3d12.h>

namespace Cyrex {
    class ConstantBuffer : public Buffer {
    public:
        size_t GetSizeInBytes() const noexcept { return m_sizeInBytes; }
    protected:
        ConstantBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource);
        virtual ~ConstantBuffer();
    private:
        size_t m_sizeInBytes;
    };
}