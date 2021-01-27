#pragma once
#include "DescriptorAllocation.h"
#include "Resource.h"
#include "d3dx12.h"
#include <mutex>
#include <unordered_map>

namespace Cyrex {
    class Device;
    class Texture : public Resource {
    public:
        void Rezize(uint32_t width, uint32_t height, uint32_t depthOrArraySize = 1);
        D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(uint32_t mip) const;
    
        bool ChechSRVSupport() const { return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE); }
        bool CheckRTVSupport() const { return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_RENDER_TARGET); }
        bool CheckDSVSupport() const { return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL); }
        bool CheckUAVSupport() const { 
            return 
                CheckFormatSupport(D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) &&
                CheckFormatSupport(D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD)              &&
                CheckFormatSupport(D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE);
        }

        static bool IsUAVCompatibleFormat(DXGI_FORMAT format);
        static bool IsSRGBFormat(DXGI_FORMAT format);
        static bool IsBGRFormat(DXGI_FORMAT format);
        static bool IsDepthFormat(DXGI_FORMAT format);

        static DXGI_FORMAT GetTypelessFormat(DXGI_FORMAT format);
        static DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT format);
        static DXGI_FORMAT GetUAVCompatableFormat(DXGI_FORMAT format);

        size_t BitsPerPixel() const noexcept;
    protected:
        Texture(
            Device& device,
            const D3D12_RESOURCE_DESC& resourceDesc,
            const D3D12_CLEAR_VALUE* clearValue = nullptr);
        Texture(
            Device& device, 
            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            const D3D12_CLEAR_VALUE* clearValue = nullptr);
        virtual ~Texture();

        void CreateViews();
    private:
        D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc(
            const D3D12_RESOURCE_DESC& resourceDesc, 
            uint32_t mipSlice, 
            uint32_t arraySlice = 0, 
            uint32_t planeSlice = 0);

        DescriptorAllocation m_renderTargetView;
        DescriptorAllocation m_depthStencilView;
        DescriptorAllocation m_shaderResourceView;
        DescriptorAllocation m_unorderedAccessView;
    };
}