#include "Adapter.h"
#include "DXException.h"

namespace wrl = Microsoft::WRL;

class MakeAdapter final : public Cyrex::Adapter {
public:
    MakeAdapter(Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter)
        : 
        Adapter(dxgiAdapter) 
    {}
    ~MakeAdapter() {};
};

Cyrex::AdapterList Cyrex::Adapter::GetAdapters(DXGI_GPU_PREFERENCE gpuPreference) {
    AdapterList adapters;

    wrl::ComPtr<IDXGIFactory6> dxgiFactory6;
    wrl::ComPtr<IDXGIAdapter>  dxgiAdapter;
    wrl::ComPtr<IDXGIAdapter4> dxgiAdapter4;

    uint32_t createFactoryFlags = 0;

#ifdef _DEBUG
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory6)));

    for (uint32_t i = 0; 
        dxgiFactory6->EnumAdapterByGpuPreference(i, gpuPreference, IID_PPV_ARGS(&dxgiAdapter)) != DXGI_ERROR_NOT_FOUND; 
        i++)
    {
        if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) {
            ThrowIfFailed(dxgiAdapter.As(&dxgiAdapter4));

            std::shared_ptr<Adapter> adapter = std::make_shared<MakeAdapter>(dxgiAdapter4);
            adapters.push_back(adapter);
        }
    }

    return adapters;
}

std::shared_ptr<Cyrex::Adapter> Cyrex::Adapter::Create(DXGI_GPU_PREFERENCE gpuPreference, bool useWarp) {
    std::shared_ptr<Adapter> adapter = nullptr;

    wrl::ComPtr<IDXGIFactory6> dxgiFactory6;
    wrl::ComPtr<IDXGIAdapter>  dxgiAdapter;
    wrl::ComPtr<IDXGIAdapter4> dxgiAdapter4;

    uint32_t createFactoryFlags = 0;
#if defined( _DEBUG )
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory6)));

    if (useWarp) {
        ThrowIfFailed(dxgiFactory6->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter)));
        ThrowIfFailed(dxgiAdapter.As(&dxgiAdapter4));
    }

    else {
        for (uint32_t i = 0;
            dxgiFactory6->EnumAdapterByGpuPreference(i, gpuPreference, IID_PPV_ARGS(&dxgiAdapter)) != DXGI_ERROR_NOT_FOUND;
            i++)
        {
            if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) {
                ThrowIfFailed(dxgiAdapter.As(&dxgiAdapter4));
                break;
            }
        }
    }

    if (dxgiAdapter4) {
        adapter = std::make_shared<MakeAdapter>(dxgiAdapter4);
    }

    return adapter;
}

const std::wstring Cyrex::Adapter::GetDescription() const {
    return m_desc.Description;
}

Cyrex::Adapter::Adapter(Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter) 
    :
    m_dxgiAdapter(dxgiAdapter)
{
    if (m_dxgiAdapter) {
        ThrowIfFailed(m_dxgiAdapter->GetDesc3(&m_desc));
    }
}