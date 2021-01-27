#include "ConstantBufferView.h"
#include "Device.h"
#include "ConstantBuffer.h"
#include "Core/Math/Math.h"
#include <cassert>

Cyrex::ConstantBufferView::ConstantBufferView(
    Device& device, 
    const std::shared_ptr<ConstantBuffer>& constantBuffer, 
    size_t offset)
    :
    m_device(device),
    m_constantBuffer(constantBuffer)
{
    assert(constantBuffer);

    const auto d3d12Device = m_device.GetD3D12Device();
    auto d3d12Resource     = m_constantBuffer->GetD3D12Resource();

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
    cbv.BufferLocation = d3d12Resource->GetGPUVirtualAddress() + offset;
    // Constant buffers must be aligned for hardware requirements.
    cbv.SizeInBytes = Math::AlignUp(
        m_constantBuffer->GetSizeInBytes(),
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

    m_descriptor = device.AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    d3d12Device->CreateConstantBufferView(&cbv, m_descriptor.GetDescriptorHandle());
}
