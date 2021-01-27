#pragma once

#include "DescriptorAllocation.h"
#include "d3dx12.h"
#include <mutex>
#include <memory>
#include <set>
#include <vector>

namespace Cyrex {
    class DescriptorAllocatorPage;
    class Device;
    class DescriptorAllocator {
    public:
        DescriptorAllocation Allocate(uint32_t numDescriptors = 1);
        void ReleaseStaleDescriptors();
    protected:
        friend class std::default_delete<DescriptorAllocator>;

        DescriptorAllocator(Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 256);
        virtual ~DescriptorAllocator();
    private:
        std::shared_ptr<DescriptorAllocatorPage> CreateAllocatorPage();

        using DescriptorHeapPool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;

        Device& m_device;
        D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;
        uint32_t m_numDescriptorsPerHeap;

        DescriptorHeapPool m_heapPool;

        //Indices of available heaps in the heap pool.
        std::set<size_t> m_availableHeaps;
        std::mutex m_allocationMutex;
    };
}
