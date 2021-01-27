#include "Mesh.h"
#include "CommandList.h"
#include "Device.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Core/Visitor.h"

namespace dx = DirectX;
namespace wrl = Microsoft::WRL;
namespace crx = Cyrex;

Cyrex::Mesh::Mesh()
    :
    m_PrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
{}

D3D12_PRIMITIVE_TOPOLOGY Cyrex::Mesh::GetPrimitiveTopology() const noexcept {
    return m_PrimitiveTopology;
}

void Cyrex::Mesh::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveToplogy) noexcept {
    m_PrimitiveTopology = primitiveToplogy;
}

std::shared_ptr<Cyrex::VertexBuffer> Cyrex::Mesh::GetVertexBuffer(uint32_t slotID) const noexcept {
    auto iter = m_vertexBuffers.find(slotID);
    auto vertexBuffer = (iter != m_vertexBuffers.end()) ? iter->second : nullptr;

    return vertexBuffer;
}

void Cyrex::Mesh::SetVertexBuffer(uint32_t slotID, const std::shared_ptr<VertexBuffer>& vertexBuffer) noexcept {
    m_vertexBuffers[slotID] = vertexBuffer;
}

std::shared_ptr<Cyrex::IndexBuffer> Cyrex::Mesh::GetIndexBuffer() const noexcept {
    return m_indexBuffer;
}

void Cyrex::Mesh::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) noexcept {
    m_indexBuffer = indexBuffer;
}

size_t Cyrex::Mesh::GetIndexCount() const noexcept {
    size_t indexCount = 0;

    if (m_indexBuffer) {
        indexCount = m_indexBuffer->GetNumIndices();
    }
    return indexCount;
}

size_t Cyrex::Mesh::GetVertexCount() const noexcept {
    size_t vertexCount = 0;

    VertexBufferMap::const_iterator iter = m_vertexBuffers.cbegin();

    if (iter != m_vertexBuffers.cend()) {
        vertexCount = iter->second->GetNumVertices();
    }

    return vertexCount;
}

std::shared_ptr<Cyrex::Material> Cyrex::Mesh::GetMaterial() const noexcept {
    return m_material;
}

void Cyrex::Mesh::SetMaterial(std::shared_ptr<Material> material) noexcept {
    m_material = material;
}

void Cyrex::Mesh::SetAABB(const DirectX::BoundingBox& aabb) noexcept {
    m_AABB = aabb;
}

const DirectX::BoundingBox& Cyrex::Mesh::GetAABB() const noexcept {
    return m_AABB;
}

void Cyrex::Mesh::Accept(IVisitor& visitor) noexcept {
    visitor.Visit(*this);
}

void Cyrex::Mesh::Render(CommandList& commandList, uint32_t instanceCount, uint32_t firstInstance) {
    commandList.SetPrimitiveTopology(GetPrimitiveTopology());

    for (auto vertexBuffer : m_vertexBuffers) {
        commandList.SetVertexBuffer(vertexBuffer.first, vertexBuffer.second);
    }

    auto indexCount  = GetIndexCount();
    auto vertexCount = GetVertexCount();

    if (indexCount > 0) {
        commandList.SetIndexBuffer(m_indexBuffer);
        commandList.DrawIndexed(indexCount, instanceCount, 0u, 0u, firstInstance);
    }
    else if (vertexCount > 0) {
        commandList.Draw(vertexCount, instanceCount, 0u, firstInstance);
    }
}
