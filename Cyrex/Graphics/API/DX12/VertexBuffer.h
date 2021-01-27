#pragma once
#include "Buffer.h"

namespace Cyrex {
    class VertexBuffer : public Buffer {
    public:
        [[nodiscard]] D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const noexcept { return m_vertexBufferView; }
        [[nodiscard]] size_t GetNumVertices() const noexcept { return m_numVertices; }
        [[nodiscard]] size_t GetVertexStride() const noexcept { return m_vertexStride; }
    protected:
        VertexBuffer(Device& device, size_t numVertices, size_t vertexStride);
        VertexBuffer(
            Device& device, 
            Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
            size_t numVertices,
            size_t vertexStride);
        virtual ~VertexBuffer();
    protected:
        void CreateVertexBufferView();
    private:
        size_t m_numVertices{0};
        size_t m_vertexStride{0};
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
    };
}
