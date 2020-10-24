#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <memory>

namespace Cyrex {
    class Resource {
    public:
        explicit Resource(const std::wstring& name = L"");
        explicit Resource(const D3D12_RESOURCE_DESC& resourceDesc,
            const D3D12_CLEAR_VALUE* clearValue = nullptr,
            const std::wstring& name = L"");
        explicit Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const std::wstring& name = L"");

        Resource(const Resource& rhs);
        Resource(Resource&& rhs) noexcept;

        Resource& operator=(const Resource& rhs);
        Resource& operator=(Resource&& rhs) noexcept;
        ~Resource() = default;
    public:
        bool IsValid() const noexcept { return m_d3d12Resource != nullptr; }
        Microsoft::WRL::ComPtr<ID3D12Resource> GetD3D12Resource() const noexcept { return m_d3d12Resource; }

        D3D12_RESOURCE_DESC GetD3D12ResourceDesc() const noexcept;

        virtual void SetD3D12Resource(
            Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource,
            const D3D12_CLEAR_VALUE* clearValue = nullptr);

        virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(
            const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const = 0;

        virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(
            const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const = 0;
        virtual void Reset();

        void SetName(const std::wstring& name);
        bool CheckFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const;
        bool CheckFormatSupport(D3D12_FORMAT_SUPPORT2 formatSupport) const;
    protected:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12Resource;
        D3D12_FEATURE_DATA_FORMAT_SUPPORT m_formatSupport;
        std::unique_ptr<D3D12_CLEAR_VALUE> m_d3d12ClearValue;
        std::wstring m_resourceName;
    private:
        void CheckFeatureSupport();
    };
}
