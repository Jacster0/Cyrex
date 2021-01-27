#include "StructuredBuffer.h"
#include "Device.h"
#include "ResourceStateTracker.h"
#include "d3dx12.h"

Cyrex::StructuredBuffer::StructuredBuffer(Device& device, size_t numElements, size_t elementSize)
    :
    Buffer(device, CD3DX12_RESOURCE_DESC::Buffer(numElements*elementSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)),
    m_numElements(numElements),
    m_elementSize(elementSize)

{
    m_counterBuffer = m_device.CreateByteAddressBuffer(4);
}

Cyrex::StructuredBuffer::StructuredBuffer(
    Device& device, 
    Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
    size_t numElements, 
    size_t elementSize)
    : 
    Buffer(device, resource), 
    m_numElements(numElements), 
    m_elementSize(elementSize)
{
    m_counterBuffer = m_device.CreateByteAddressBuffer(4);
}
