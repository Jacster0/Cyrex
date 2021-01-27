#include "Resource.h"
#include "d3dx12.h"
#include "DXException.h"
#include "ResourceStateTracker.h"
#include "Device.h"

D3D12_RESOURCE_DESC Cyrex::Resource::GetD3D12ResourceDesc() const noexcept {
    D3D12_RESOURCE_DESC resDesc = {};

    if (m_d3d12Resource) {
        resDesc = m_d3d12Resource->GetDesc();
    }

    return resDesc;
}

void Cyrex::Resource::SetName(const std::wstring& name) {
    m_resourceName = name;

    if (m_d3d12Resource and !m_resourceName.empty()) {
        m_d3d12Resource->SetName(m_resourceName.c_str());
    }
}

bool Cyrex::Resource::CheckFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const {
    return (m_formatSupport.Support1 & formatSupport) != 0;
}

bool Cyrex::Resource::CheckFormatSupport(D3D12_FORMAT_SUPPORT2 formatSupport) const {
    return (m_formatSupport.Support2 & formatSupport) != 0;
}

Cyrex::Resource::Resource(Device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue)
    :
    m_device(device)
{
    const auto d3d12Device = m_device.GetD3D12Device();

    if (clearValue) {
        m_d3d12ClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*clearValue);
    }

    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    ThrowIfFailed(d3d12Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        m_d3d12ClearValue.get(),
        IID_PPV_ARGS(&m_d3d12Resource)));

    ResourceStateTracker::AddGlobalResourceState(m_d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON);
    CheckFeatureSupport();
}

Cyrex::Resource::Resource(
    Device& device, 
    Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
    const D3D12_CLEAR_VALUE* clearValue)
    :
    m_device(device),
    m_d3d12Resource(resource)

{
    if (clearValue) {
        m_d3d12ClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*clearValue);
    }
    CheckFeatureSupport();
}

void Cyrex::Resource::CheckFeatureSupport() {
    const auto device = m_device.GetD3D12Device();

    const auto desc = m_d3d12Resource->GetDesc();
    m_formatSupport.Format = desc.Format;

    ThrowIfFailed(device->CheckFeatureSupport(
        D3D12_FEATURE_FORMAT_SUPPORT,
        &m_formatSupport,
        sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT)));
}
