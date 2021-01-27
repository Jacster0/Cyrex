#include "UploadBuffer.h"
#include "d3dx12.h"
#include "DXException.h"
#include "Core/Math/Math.h"
#include "Device.h"

namespace wrl = Microsoft::WRL;

Cyrex::UploadBuffer::UploadBuffer(Device& device, size_t pageSize)
    :
    m_device(device),
    m_pageSize(pageSize)
{}

Cyrex::UploadBuffer::~UploadBuffer()
{
}

Cyrex::UploadBuffer::Allocation Cyrex::UploadBuffer::Allocate(size_t sizeInBytes, size_t alignment) {
    if (sizeInBytes > m_pageSize) {
        throw std::bad_alloc();
    }

    if (!m_currentPage || m_currentPage->HasSpace(sizeInBytes, alignment)) {
        m_currentPage = RequestPage();
    }

    return m_currentPage->Allocate(sizeInBytes, alignment);
}

void Cyrex::UploadBuffer::Reset() {
    m_currentPage = nullptr;
    m_availablePages = m_pagePool;

    for (auto page : m_availablePages) {
        page->Reset();
    }
}

std::shared_ptr<Cyrex::UploadBuffer::Page> Cyrex::UploadBuffer::RequestPage() {
    std::shared_ptr<Page> page;

    if (!m_availablePages.empty()) {
        page = m_availablePages.front();
        m_availablePages.pop_front();
    }
    else {
        page = std::make_shared<Page>(m_device, m_pageSize);
        m_pagePool.push_back(page);
    }

    return page;
}

Cyrex::UploadBuffer::Page::Page(Device& device, size_t sizeInBytes)
    :
    m_device(device),
    m_pageSize(sizeInBytes),
    m_offset(0),
    m_cpuPtr(nullptr),
    m_gpuPtr(D3D12_GPU_VIRTUAL_ADDRESS(0))
{
    const auto& d3d12Device = m_device.GetD3D12Device();

    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto buffer = CD3DX12_RESOURCE_DESC::Buffer(m_pageSize);

    ThrowIfFailed(d3d12Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &buffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_d3d12Resource)
    ));

    m_d3d12Resource->SetName(L"Upload Buffer (Page)");

    m_gpuPtr = m_d3d12Resource->GetGPUVirtualAddress();
    m_d3d12Resource->Map(0, nullptr, &m_cpuPtr);
}

Cyrex::UploadBuffer::Page::~Page() {
    m_d3d12Resource->Unmap(0, nullptr);
    m_cpuPtr = nullptr;
    m_gpuPtr = D3D12_GPU_VIRTUAL_ADDRESS(0);
}

bool Cyrex::UploadBuffer::Page::HasSpace(size_t sizeInBytes, size_t alignment) const {
    size_t alignedSize   = Math::AlignUp(sizeInBytes, alignment);
    size_t alignedOffset = Math::AlignUp(m_offset, alignment);

    return alignedOffset + alignedSize <= m_pageSize;
}

Cyrex::UploadBuffer::Allocation Cyrex::UploadBuffer::Page::Allocate(size_t sizeInBytes, size_t alignment) {
    if (!HasSpace(sizeInBytes, alignment)) {
        throw std::bad_alloc();
    }
    size_t alignedSize = 0;

    Allocation alloc;
    alloc.CPU = static_cast<uint8_t*>(m_cpuPtr) + m_offset;
    alloc.GPU = m_gpuPtr + m_offset;

    m_offset += alignedSize;
    return alloc;
}

void Cyrex::UploadBuffer::Page::Reset() {
    m_offset = 0;
}
