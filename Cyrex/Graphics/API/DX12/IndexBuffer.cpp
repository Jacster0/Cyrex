#include "IndexBuffer.h"
#include "d3dx12.h"
#include <cassert>

Cyrex::IndexBuffer::IndexBuffer(Device& device, size_t numIndices, DXGI_FORMAT indexFormat)
    :
    Buffer(device,CD3DX12_RESOURCE_DESC::Buffer(numIndices * (indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4))),
    m_numIndices(numIndices),
    m_indexFormat(indexFormat),
    m_indexBufferView{}
{
    assert(indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT);
    CreateIndexBufferView();
}

Cyrex::IndexBuffer::IndexBuffer(
    Device& device, 
    Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
    size_t numIndices, 
    DXGI_FORMAT indexFormat)
    :
    Buffer(device,resource),
    m_numIndices(numIndices),
    m_indexFormat(indexFormat),
    m_indexBufferView{}
{
    assert(indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT);
    CreateIndexBufferView();
}

void Cyrex::IndexBuffer::CreateIndexBufferView() {
    uint32_t bufferSize = m_numIndices * (m_indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4);

    m_indexBufferView.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes    = bufferSize;
    m_indexBufferView.Format         = m_indexFormat;
}
