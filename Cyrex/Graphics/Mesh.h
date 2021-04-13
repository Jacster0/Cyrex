#pragma once
#include <memory>
#include <map>
#include <d3d12.h>
#include <DirectXCollision.h>

namespace Cyrex {
    class CommandList;
    class IndexBuffer;
    class VertexBuffer;
    class Material;
    class IVisitor;

    class Mesh {
    public:
        using VertexBufferMap = std::map<uint32_t, std::shared_ptr<VertexBuffer>>;

        Mesh();
        Mesh(const Mesh& rhs) = delete;
        Mesh& operator=(const Mesh& rhs) = delete;
        virtual ~Mesh() = default;

        [[nodiscard]] D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const noexcept;
        void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveToplogy) noexcept;

        std::shared_ptr<VertexBuffer> GetVertexBuffer(uint32_t slotID) const noexcept;
        void SetVertexBuffer(uint32_t slotID, const std::shared_ptr<VertexBuffer>& vertexBuffer) noexcept;
        
        [[nodiscard]] const VertexBufferMap& GetVertexBuffers() const noexcept { return m_vertexBuffers; }

        [[nodiscard]] std::shared_ptr<IndexBuffer> GetIndexBuffer() const noexcept;
        void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) noexcept;

        [[nodiscard]] size_t GetIndexCount() const noexcept;
        [[nodiscard]] size_t GetVertexCount() const noexcept;

        [[nodiscard]] std::shared_ptr<Material> GetMaterial() const noexcept;
        void SetMaterial(std::shared_ptr<Material> material) noexcept;
       
        [[nodiscard]] const DirectX::BoundingBox& GetAABB() const noexcept;

        void SetAABB(const DirectX::BoundingBox& aabb) noexcept;
       
        void Accept(IVisitor& visitor) noexcept;
        void Render(CommandList& commandList, uint32_t instanceCount = 1, uint32_t firstInstance = 0);
    private:
        VertexBufferMap m_vertexBuffers;
        std::shared_ptr<IndexBuffer> m_indexBuffer;

        std::shared_ptr<Material> m_material;
        D3D12_PRIMITIVE_TOPOLOGY m_PrimitiveTopology;

        DirectX::BoundingBox m_AABB;
    };
};