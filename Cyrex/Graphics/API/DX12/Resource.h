#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <memory>

namespace Cyrex {
    class Device;
    class Resource {
    public:
        Microsoft::WRL::ComPtr<ID3D12Resource> GetD3D12Resource() const noexcept { return m_d3d12Resource; }
        D3D12_RESOURCE_DESC GetD3D12ResourceDesc() const noexcept;

        void SetName(const std::wstring& name);
        const std::wstring& GetName() const { return m_resourceName; }

        bool CheckFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const;
        bool CheckFormatSupport(D3D12_FORMAT_SUPPORT2 formatSupport) const;
    protected:
        Resource(Device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr);
        Resource(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);
        virtual ~Resource() = default;
    protected:
        Device& m_device; 

        Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12Resource;
        D3D12_FEATURE_DATA_FORMAT_SUPPORT m_formatSupport;
        std::unique_ptr<D3D12_CLEAR_VALUE> m_d3d12ClearValue;
        std::wstring m_resourceName;
    private:
        void CheckFeatureSupport();
    };
}
