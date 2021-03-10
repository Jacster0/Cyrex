#include "TextureManager.h"

#include "Graphics/API/DX12/CommandList.h"
#include "Graphics/API/DX12/Texture.h"
#include "Extern/DirectXTex/DirectXTex/DirectXTex.h"
#include "Graphics/API/DX12/Device.h"
#include "Graphics/API/DX12/DXException.h"
#include "Graphics/API/DX12/d3dx12.h"
#include "Graphics/API/DX12/ResourceStateTracker.h"

#include <wrl.h>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;
namespace wrl = Microsoft::WRL;
using namespace Cyrex;
using namespace DirectX;

std::map<std::wstring, ID3D12Resource*> TextureManager::ms_textureCache;
std::mutex TextureManager::ms_textureCacheMutex;

std::shared_ptr<Texture> TextureManager::LoadTextureFromFile(CommandList& commandList, const std::wstring fileName, bool sRGB) {
    std::shared_ptr<Texture> texture;
    fs::path filePath(fileName);

    if (!fs::exists(filePath)) {
        throw std::exception("File not found");
    }

    std::lock_guard<std::mutex> lock(ms_textureCacheMutex);
    const auto iter = ms_textureCache.find(fileName);

    if (iter != ms_textureCache.end()) {
        texture = commandList.GetDevice().CreateTexture(iter->second);
    }

    else {
        TexMetadata metadata;
        ScratchImage scratchImage;

        const auto fileExtension = filePath.extension();

        if (fileExtension == ".dds") {
            ThrowIfFailed(LoadFromDDSFile(fileName.c_str(), DDS_FLAGS_FORCE_RGB, &metadata, scratchImage));
        }
        else if (fileExtension == ".hdr") {
            ThrowIfFailed(LoadFromHDRFile(fileName.c_str(), &metadata, scratchImage));
        }
        else if (fileExtension == ".tga") {
            ThrowIfFailed(LoadFromTGAFile(fileName.c_str(), &metadata, scratchImage));
        }
        else {
            ThrowIfFailed(LoadFromWICFile(fileName.c_str(), WIC_FLAGS_FORCE_RGB, &metadata, scratchImage));
        }

        if (sRGB) {
            metadata.format = MakeSRGB(metadata.format);
        }

        D3D12_RESOURCE_DESC textureDesc = {};

        switch (metadata.dimension)
        {
        case TEX_DIMENSION_TEXTURE1D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(
                metadata.format,
                static_cast<uint64_t>(metadata.width),
                static_cast<uint16_t>(metadata.arraySize));
            break;
        case TEX_DIMENSION_TEXTURE2D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                metadata.format,
                static_cast<uint64_t>(metadata.width),
                static_cast<uint32_t>(metadata.height),
                static_cast<uint16_t>(metadata.arraySize));
            break;
        case TEX_DIMENSION_TEXTURE3D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(
                metadata.format,
                static_cast<uint64_t>(metadata.width),
                static_cast<uint32_t>(metadata.height),
                static_cast<uint16_t>(metadata.depth));
            break;
        default:
            throw std::exception("Invalid texture dimension");
        }

        const auto d3d12Device = commandList.GetDevice().GetD3D12Device();
        wrl::ComPtr<ID3D12Resource> textureResource;

        auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ThrowIfFailed(d3d12Device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&textureResource)));

        texture = commandList.GetDevice().CreateTexture(textureResource);
        texture->SetName(fileName);

        //Update the global state tracker
        ResourceStateTracker::AddGlobalResourceState(textureResource.Get(), D3D12_RESOURCE_STATE_COMMON);

        std::vector<D3D12_SUBRESOURCE_DATA> subResources(scratchImage.GetImageCount());
        const Image* pImages = scratchImage.GetImages();

        for (int i = 0; i < scratchImage.GetImageCount(); i++) {
            auto& subResource      = subResources[i];
            subResource.RowPitch   = pImages[i].rowPitch;
            subResource.SlicePitch = pImages[i].slicePitch;
            subResource.pData      = pImages[i].pixels;
        }

        commandList.CopyTextureSubresource(texture, 0, static_cast<uint32_t>(subResources.size()), subResources.data());

        if (subResources.size() < textureResource->GetDesc().MipLevels) {
            commandList.GenerateMips(texture);
        }

        ms_textureCache[fileName] = textureResource.Get();
    }

    return texture;
}

void TextureManager::ClearTexture(CommandList& commandList, const std::shared_ptr<Texture>& texture, const DirectX::XMVECTORF32 clearColor) {
    assert(texture);

    commandList.TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
    commandList.GetD3D12CommandList()->ClearRenderTargetView(texture->GetRenderTargetView(), clearColor, 0, nullptr);
    commandList.TrackResource(texture);
}

void Cyrex::TextureManager::ClearDepthStencilTexture(
    CommandList& commandList, 
    const std::shared_ptr<Texture>& texture, 
    D3D12_CLEAR_FLAGS clearFlags, 
    float depth, 
    uint8_t stencil)
{
    assert(texture);
    
    commandList.TransitionBarrier(texture, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
    commandList.GetD3D12CommandList()->ClearDepthStencilView(texture->GetDepthStencilView(), clearFlags, depth, stencil, 0, nullptr);
    commandList.TrackResource(texture);
}
