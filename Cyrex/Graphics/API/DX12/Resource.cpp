#include "Resource.h"
#include "d3dx12.h"
#include "Core/Application.h"
#include "DXException.h"
#include "ResourceStateTracker.h"

Cyrex::Resource::Resource(const std::wstring& name)
    :
    m_resourceName(name),
    m_formatSupport({})
{}

Cyrex::Resource::Resource(
    const D3D12_RESOURCE_DESC& resourceDesc, 
    const D3D12_CLEAR_VALUE* clearValue, 
    const std::wstring& name)
{
    if (clearValue) {
        m_d3d12ClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*clearValue);
    }

    const auto device = Application::Get().GetDevice();

    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        m_d3d12ClearValue.get(),
        IID_PPV_ARGS(&m_d3d12Resource)));

    ResourceStateTracker::AddGlobalResourceState(m_d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON);
    CheckFeatureSupport();
    SetName(name);
}

Cyrex::Resource::Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const std::wstring& name)
    :
    m_d3d12Resource(resource),
    m_formatSupport({})
{
    CheckFeatureSupport();
    SetName(name);
}

Cyrex::Resource::Resource(const Resource& rhs)
    :
    m_d3d12Resource(rhs.m_d3d12Resource),
    m_formatSupport(rhs.m_formatSupport),
    m_resourceName(rhs.m_resourceName),
    m_d3d12ClearValue(std::make_unique<D3D12_CLEAR_VALUE>(*rhs.m_d3d12ClearValue))
{}

Cyrex::Resource::Resource(Resource&& rhs) noexcept
    :

    m_d3d12Resource(std::move(rhs.m_d3d12Resource)),
    m_formatSupport(rhs.m_formatSupport),
    m_resourceName(std::move(rhs.m_resourceName)),
    m_d3d12ClearValue(std::move(m_d3d12ClearValue))
{}

Cyrex::Resource& Cyrex::Resource::operator=(const Resource& rhs) {
    if (this != &rhs) {
        m_d3d12Resource = rhs.m_d3d12Resource;
        m_formatSupport = rhs.m_formatSupport;
        m_resourceName  = rhs.m_resourceName;

        if (rhs.m_d3d12ClearValue) {
            m_d3d12ClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*rhs.m_d3d12ClearValue);
        }
    }

    return *this;
}

Cyrex::Resource& Cyrex::Resource::operator=(Resource&& rhs) noexcept {
    if (this != &rhs) {
        m_d3d12Resource   = std::move(rhs.m_d3d12Resource);
        m_formatSupport   = rhs.m_formatSupport;
        m_resourceName    = std::move(rhs.m_resourceName);
        m_d3d12ClearValue = std::move(rhs.m_d3d12ClearValue);

        rhs.Reset();
    }

    return *this;
}

D3D12_RESOURCE_DESC Cyrex::Resource::GetD3D12ResourceDesc() const noexcept {
    D3D12_RESOURCE_DESC resDesc = {};

    if (m_d3d12Resource) {
        resDesc = m_d3d12Resource->GetDesc();
    }

    return resDesc;
}

void Cyrex::Resource::SetD3D12Resource(
    Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource, 
    const D3D12_CLEAR_VALUE* clearValue)
{
    m_d3d12Resource = d3d12Resource;

    if (m_d3d12ClearValue) {
        m_d3d12ClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*clearValue);
    }
    else {
        m_d3d12ClearValue.reset();
    }

    CheckFeatureSupport();
    SetName(m_resourceName);
}

void Cyrex::Resource::Reset() {
    m_d3d12Resource.Reset();
    m_formatSupport = {};
    m_d3d12ClearValue.reset();
    m_resourceName.clear();
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

void Cyrex::Resource::CheckFeatureSupport() {
    if (m_d3d12Resource) {
        const auto desc   = m_d3d12Resource->GetDesc();
        const auto device = Application::Get().GetDevice();

        m_formatSupport.Format = desc.Format;

        ThrowIfFailed(device->CheckFeatureSupport(
            D3D12_FEATURE_FORMAT_SUPPORT,
            &m_formatSupport,
            sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT)));
    }
    else {
        m_formatSupport = {};
    }
}
