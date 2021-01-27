#pragma once

#include "DescriptorAllocation.h"
#include <d3d12.h>
#include <memory>

namespace Cyrex {
    class Device;
    class Resource;

    class ShaderResourceView {
    public:
        std::shared_ptr<Resource> GetResource() const noexcept { return m_resource; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const { return m_descriptor.GetDescriptorHandle(); }
    protected:
        ShaderResourceView(Device& device, const std::shared_ptr<Resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr);
        virtual ~ShaderResourceView() = default;
    private:
        Device& m_device;
        std::shared_ptr<Resource> m_resource;
        DescriptorAllocation m_descriptor;
    };
}