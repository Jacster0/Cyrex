#pragma once
#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <d3d12.h>
#include <DirectXMath.h>

namespace Cyrex {
    class CommandList;
    class Texture;
    class TextureManager {
    public:
        static std::shared_ptr<Texture> LoadTextureFromFile(CommandList& commandList, const std::string fileName, bool sRGB);
        static void ClearTexture(CommandList& commandList, const std::shared_ptr<Texture>& texture, const DirectX::XMVECTORF32 clearColor);
        static void ClearDepthStencilTexture(
            CommandList& commandList,
            const std::shared_ptr<Texture>& texture,
            D3D12_CLEAR_FLAGS clearFlags,
            float depth = 1.0f,
            uint8_t stencil = 0);
    private:
        static std::map<std::wstring, ID3D12Resource*> ms_textureCache;
        static std::mutex ms_textureCacheMutex;
    };
}