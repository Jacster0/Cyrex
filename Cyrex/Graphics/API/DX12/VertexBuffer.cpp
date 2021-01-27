#include "VertexBuffer.h"
#include "d3dx12.h"

Cyrex::VertexBuffer::VertexBuffer(Device& device, size_t numVertices, size_t vertexStride)
    :
    Buffer(device, CD3DX12_RESOURCE_DESC::Buffer(numVertices * vertexStride)),
    m_numVertices(numVertices),
    m_vertexStride(vertexStride)
{
    CreateVertexBufferView();
}

Cyrex::VertexBuffer::VertexBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride)
    :
    Buffer(device, resource),
    m_numVertices(numVertices),
    m_vertexStride(vertexStride)
{
    CreateVertexBufferView();
}

Cyrex::VertexBuffer::~VertexBuffer() {}

void Cyrex::VertexBuffer::CreateVertexBufferView() {
    m_vertexBufferView.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes    = static_cast<uint32_t>(m_numVertices * m_vertexStride);
    m_vertexBufferView.StrideInBytes  = static_cast<uint32_t>(m_vertexStride);
}
