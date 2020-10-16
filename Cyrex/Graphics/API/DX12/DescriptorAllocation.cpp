#include "DescriptorAllocation.h"
#include "DescriptorAllocatorPage.h"
#include "Core/Application.h"
#include <cassert>

Cyrex::DescriptorAllocation::DescriptorAllocation()
    :
    m_descriptor{0},
    m_numHandles(0),
    m_descriptorSize(0),
    m_page(nullptr)
{}

Cyrex::DescriptorAllocation::DescriptorAllocation(
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor,
    uint32_t numHandles,
    uint32_t descriptorSize,
    std::shared_ptr<DescriptorAllocatorPage> page)
    :
    m_descriptor{ descriptor },
    m_numHandles(numHandles),
    m_descriptorSize(descriptorSize),
    m_page(page)
{}

Cyrex::DescriptorAllocation::~DescriptorAllocation() {
    Free();
}

Cyrex::DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& allocation) noexcept
    :
    m_descriptor(allocation.m_descriptor),
    m_numHandles(allocation.m_numHandles),
    m_descriptorSize(allocation.m_descriptorSize),
    m_page(allocation.m_page)
{
    allocation.m_descriptor.ptr = 0;
    allocation.m_numHandles     = 0;
    allocation.m_descriptorSize = 0;
}

Cyrex::DescriptorAllocation& Cyrex::DescriptorAllocation::operator=(DescriptorAllocation&& other) noexcept {
    // Free this descriptor if it points to anything.
    Free();

    m_descriptor     = other.m_descriptor;
    m_numHandles     = other.m_numHandles;
    m_descriptorSize = other.m_descriptorSize;
    m_page           = std::move(other.m_page);

    other.m_descriptor.ptr = 0;
    other.m_numHandles     = 0;
    other.m_descriptorSize = 0;

    return *this;
}

bool  Cyrex::DescriptorAllocation::IsNull() const {
    return m_descriptor.ptr == 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE  Cyrex::DescriptorAllocation::GetDescriptorHandle(uint32_t offset) const {
    assert(offset < m_numHandles);
    return { m_descriptor.ptr + (m_descriptorSize * offset) };
}

uint32_t  Cyrex::DescriptorAllocation::GetNumHandles() const {
    return m_numHandles;
}

std::shared_ptr< Cyrex::DescriptorAllocatorPage>  Cyrex::DescriptorAllocation::GetDescriptorAllocatorPage() const {
    return m_page;
}

void Cyrex::DescriptorAllocation::Free() {
    if (!IsNull() and m_page) {
        m_page->Free(std::move(*this), Application::FrameCount());

        m_descriptor.ptr = 0;
        m_numHandles = 0;
        m_descriptorSize = 0;
        m_page.reset();
    }
}
