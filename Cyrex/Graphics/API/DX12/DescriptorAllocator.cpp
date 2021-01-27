#include "DescriptorAllocator.h"
#include "DescriptorAllocatorPage.h"
#include "Device.h"
#include <algorithm>

class DescriptorAllocatorPageContext final : public Cyrex::DescriptorAllocatorPage {
public:
    DescriptorAllocatorPageContext(Cyrex::Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
        : DescriptorAllocatorPage(device, type, numDescriptors)
    {}

    ~DescriptorAllocatorPageContext() {}
}; 

Cyrex::DescriptorAllocation Cyrex::DescriptorAllocator::Allocate(uint32_t numDescriptors) {
    std::lock_guard<std::mutex> lock(m_allocationMutex);

    DescriptorAllocation allocation;

    auto iter = m_availableHeaps.begin();

    while (iter != m_availableHeaps.end()) {
        auto allocatorPage = m_heapPool[*iter];

        allocation = allocatorPage->Allocate(numDescriptors);

        if (allocatorPage->NumFreeHandles() == 0) {
            iter = m_availableHeaps.erase(iter);
        }
        else {
            ++iter;
        }

        // A valid allocation has been found.
        if (!allocation.IsNull()) {
            break;
        }
    }

    // No available heap could satisfy the requested number of descriptors.
    if (allocation.IsNull()) {
        m_numDescriptorsPerHeap = std::max(m_numDescriptorsPerHeap, numDescriptors);
        auto newPage = CreateAllocatorPage();

        allocation = newPage->Allocate(numDescriptors);
    }

    return allocation;
}

void Cyrex::DescriptorAllocator::ReleaseStaleDescriptors() {
    std::lock_guard<std::mutex> lock(m_allocationMutex);

    for (size_t i = 0; i < m_heapPool.size(); i++) {
        auto page = m_heapPool[i];

        page->ReleaseStaleDescriptors();

        if (page->NumFreeHandles() > 0) {
            m_availableHeaps.insert(i);
        }
    }
}

Cyrex::DescriptorAllocator::DescriptorAllocator(Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap)
    :
    m_device(device),
    m_heapType(type),
    m_numDescriptorsPerHeap(numDescriptorsPerHeap)
{}

Cyrex::DescriptorAllocator::~DescriptorAllocator() {}

std::shared_ptr<Cyrex::DescriptorAllocatorPage> Cyrex::DescriptorAllocator::CreateAllocatorPage() {
    auto newPage = std::make_shared<DescriptorAllocatorPageContext>(m_device, m_heapType, m_numDescriptorsPerHeap);

    m_heapPool.emplace_back(newPage);
    m_availableHeaps.insert(m_heapPool.size() - 1);
    return newPage;
}
