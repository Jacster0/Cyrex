#pragma once
#include <DirectXMath.h>  
#include <d3d12.h>   
#include <cstdint>
#include <memory>  
#include <vector>

namespace Cyrex {
    enum AttachmentPoint {
        Color0,
        Color1,
        Color2,
        Color3,
        Color4,
        Color5,
        Color6,
        Color7,
        DepthStencil,
        NumAttachmentPoints,
    };
    
    class Texture;
    using RenderTargetList = std::vector<std::shared_ptr<Texture>>;

    class RenderTarget {
    public:
        RenderTarget();

        RenderTarget(const RenderTarget& rhs) = default;
        RenderTarget(RenderTarget && rhs)     = default;

        RenderTarget& operator=(const RenderTarget & rhs) = default;
        RenderTarget& operator=(RenderTarget && rhs)      = default;
    public:
        void AttachTexture(AttachmentPoint attachmentPoint, std::shared_ptr<Texture> texture);
        std::shared_ptr<Texture> GetTexture(AttachmentPoint attachmentPoint) const;

        void Resize(DirectX::XMUINT2 size);
        void Resize(uint32_t width, uint32_t height);

        DirectX::XMUINT2 GetSize() const noexcept { return m_size; }
        uint32_t         GetWidth() const noexcept { return m_size.x; }
        uint32_t         GetHeight() const noexcept { return m_size.y; }
    public:
        D3D12_VIEWPORT GetViewPort(
            DirectX::XMFLOAT2 scale = { 1.0f, 1.0f }, 
            DirectX::XMFLOAT2 bias = { 0.0f, 0.0f },
            float minDepth = 0.0f, 
            float maxDepth = 1.0f) const;
        const RenderTargetList& GetTextures() const;
        D3D12_RT_FORMAT_ARRAY GetRenderTargetFormats() const;
        DXGI_FORMAT GetDepthStencilFormat() const;
        DXGI_SAMPLE_DESC GetSampleDesc() const;
    public:
        void Reset() { m_textures = RenderTargetList(AttachmentPoint::NumAttachmentPoints); }
    private:
        RenderTargetList m_textures{ AttachmentPoint::NumAttachmentPoints };
        DirectX::XMUINT2 m_size{ 0,0 };
    };
}