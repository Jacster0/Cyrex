#pragma once

#include "Platform/Windows/CrxWindow.h"
#include <d3d12.h>
#include <memory>

namespace Cyrex {
    class DescriptorAllocatorPage;
    class DescriptorAllocation {
    public:
        // Creates a NULL descriptor.
        DescriptorAllocation();
        DescriptorAllocation(
            D3D12_CPU_DESCRIPTOR_HANDLE descriptor,
            uint32_t numHandles,
            uint32_t descriptorSize,
            std::shared_ptr<DescriptorAllocatorPage> page);

        ~DescriptorAllocation();

        DescriptorAllocation(const DescriptorAllocation&) = delete;
        DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

        DescriptorAllocation(DescriptorAllocation&& allocation) noexcept;
        DescriptorAllocation& operator=(DescriptorAllocation&& other) noexcept;
    public:
        bool IsNull() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(uint32_t offset = 0) const;
        uint32_t GetNumHandles() const;
        std::shared_ptr<DescriptorAllocatorPage> GetDescriptorAllocatorPage() const;
    private:
        void Free();
    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;
        uint32_t m_numHandles;
        uint32_t m_descriptorSize;
        std::shared_ptr<DescriptorAllocatorPage> m_page;
    };
}
