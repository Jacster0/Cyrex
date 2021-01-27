#include "ShaderResourceView.h"
#include "Device.h"
#include "Resource.h"
#include <cassert>

Cyrex::ShaderResourceView::ShaderResourceView(
    Device& device, 
    const std::shared_ptr<Resource>& resource, 
    const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
    :
    m_device(device),
    m_resource(resource)
{
    assert(resource || srv);

    const auto d3d12Resource = m_resource ? m_resource->GetD3D12Resource() : nullptr;
    const auto d3d12Device   = m_device.GetD3D12Device();

    m_descriptor = m_device.AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    d3d12Device->CreateShaderResourceView(d3d12Resource.Get(), srv, m_descriptor.GetDescriptorHandle());
}
