#pragma once
#include "DescriptorAllocation.h"
#include <d3d12.h>
#include <memory>

namespace Cyrex {
    class Device;
    class Resource;

    class UnorderedAccessView {
    public:
        std::shared_ptr<Resource> GetResource() const noexcept { return m_resource; }
        std::shared_ptr<Resource> GetCounterResource() const noexcept { return m_counterResource; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const { return m_descriptor.GetDescriptorHandle(); }
    protected:
        UnorderedAccessView(
            Device& device, 
            const std::shared_ptr<Resource>& resource,
            const std::shared_ptr<Resource>& counterResource = nullptr,
            const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr);
        virtual ~UnorderedAccessView() = default;
    private:
        Device& m_device;
        std::shared_ptr<Resource> m_resource;
        std::shared_ptr<Resource> m_counterResource;
        DescriptorAllocation      m_descriptor;
    };
 }