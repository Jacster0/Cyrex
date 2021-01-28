#include "RenderTarget.h"
#include "Texture.h"

Cyrex::RenderTarget::RenderTarget()
    :
    m_textures(AttachmentPoint::NumAttachmentPoints),
    m_size(0,0)
{}

void Cyrex::RenderTarget::AttachTexture(AttachmentPoint attachmentPoint, std::shared_ptr<Texture> texture) {
    m_textures.at(attachmentPoint) = texture;

    if (texture && texture->GetD3D12Resource()) {
        const auto desc = texture->GetD3D12ResourceDesc();

        m_size.x = static_cast<uint32_t>(desc.Width);
        m_size.y = static_cast<uint32_t>(desc.Height);
    }
}

std::shared_ptr<Cyrex::Texture> Cyrex::RenderTarget::GetTexture(AttachmentPoint attachmentPoint) const {
    return m_textures.at(attachmentPoint);
}

void Cyrex::RenderTarget::Resize(DirectX::XMUINT2 size) {
    m_size = size;
    Resize(m_size.x, m_size.y);
}

void Cyrex::RenderTarget::Resize(uint32_t width, uint32_t height) {
    for (auto texure : m_textures) {
        if (texure) {
            texure->Rezize(width, height);
        }
    }
}

D3D12_VIEWPORT Cyrex::RenderTarget::GetViewPort(
    DirectX::XMFLOAT2 scale, 
    DirectX::XMFLOAT2 bias, 
    float minDepth, 
    float maxDepth) const 
{
    uint64_t width = 0;
    uint32_t height = 0;

    for (int i = AttachmentPoint::Color0; i <= AttachmentPoint::Color7; i++) {
        if (const auto texture = m_textures.at(i)) {
            const auto desc = texture->GetD3D12ResourceDesc();
            width  = std::max(width, desc.Width);
            height = std::max(height, desc.Height);
        }
    }

    D3D12_VIEWPORT viewPort = {
        (width  * bias.x),
        (height * bias.y),
        (width  * scale.x),
        (height * scale.y),
        minDepth,
        maxDepth
    };

    return viewPort;
}

const Cyrex::RenderTargetList& Cyrex::RenderTarget::GetTextures() const {
    return m_textures;
}

D3D12_RT_FORMAT_ARRAY Cyrex::RenderTarget::GetRenderTargetFormats() const {
    D3D12_RT_FORMAT_ARRAY rtvFormats = {};

    for (int i = AttachmentPoint::Color0; i <= AttachmentPoint::Color7; i++) {
        if (const auto texture = m_textures.at(i)) {
            rtvFormats.RTFormats[rtvFormats.NumRenderTargets++] = texture->GetD3D12ResourceDesc().Format;
        }
    }

    return rtvFormats;
}

DXGI_FORMAT Cyrex::RenderTarget::GetDepthStencilFormat() const {
    DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN;

    if (const auto depthStencilTexture = m_textures[AttachmentPoint::DepthStencil]) {
        dsvFormat = depthStencilTexture->GetD3D12ResourceDesc().Format;
    }
    
    return dsvFormat;
}

DXGI_SAMPLE_DESC Cyrex::RenderTarget::GetSampleDesc() const {
    DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };
    for (int i = AttachmentPoint::Color0; i <= AttachmentPoint::Color7; i++) {
        if (const auto texure = m_textures.at(i)) {
            sampleDesc = texure->GetD3D12ResourceDesc().SampleDesc;
            break;
        }
    }

    return sampleDesc;
}
