#pragma once
#include "DescriptorAllocation.h"

#include <d3d12.h>
#include <memory>

namespace Cyrex {
    class ConstantBuffer;
    class Device;

    class ConstantBufferView {
    public:
        std::shared_ptr<ConstantBuffer> GetConstantBuffer() const noexcept { return m_constantBuffer; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const { return m_descriptor.GetDescriptorHandle(); }
    protected:
        ConstantBufferView(Device& device,
            const std::shared_ptr<ConstantBuffer>& constantBuffer,
            size_t offset = 0);
        virtual ~ConstantBufferView() = default;
    private:
        Device& m_device;
        std::shared_ptr<ConstantBuffer> m_constantBuffer;
        DescriptorAllocation            m_descriptor;
    };
}