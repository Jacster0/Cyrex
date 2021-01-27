#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <string>

#include <memory>
#include <vector>

namespace Cyrex {
    class Adapter;
    using AdapterList = std::vector<std::shared_ptr<Adapter>>;

    class Adapter {
    public:
        static AdapterList GetAdapters(DXGI_GPU_PREFERENCE gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE);
        static std::shared_ptr<Adapter> Create(
            DXGI_GPU_PREFERENCE gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            bool useWarp = false);
        Microsoft::WRL::ComPtr<IDXGIAdapter4> GetDXGIAdapter() const { return m_dxgiAdapter; }
        const std::wstring GetDescription() const;
    protected:
        Adapter(Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter);
        virtual ~Adapter() = default;
    private:
        Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgiAdapter;
        DXGI_ADAPTER_DESC3 m_desc{ 0 };
    };
}
