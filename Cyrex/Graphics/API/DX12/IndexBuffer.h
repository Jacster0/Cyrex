#pragma once

#include "Buffer.h"

namespace Cyrex {
    class IndexBuffer : public Buffer {
    public:
        size_t GetNumIndices() const noexcept { return m_numIndices; }
        DXGI_FORMAT GetIndexFormat() const noexcept { return m_indexFormat; }
        D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const noexcept { return m_indexBufferView; }
    protected:
        IndexBuffer(Device& device, size_t numIndices, DXGI_FORMAT indexFormat);
        IndexBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource,size_t numIndices, DXGI_FORMAT indexFormat);
        virtual ~IndexBuffer() = default;

        void CreateIndexBufferView();
    private:
        size_t m_numIndices;
        DXGI_FORMAT m_indexFormat;
        D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    };
}