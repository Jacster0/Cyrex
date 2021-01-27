#include "ByteAddressBuffer.h"
#include "Device.h"

Cyrex::ByteAddressBuffer::ByteAddressBuffer(Device& device, const D3D12_RESOURCE_DESC& resDesc)
    :
    Buffer(device,resDesc)
{}

Cyrex::ByteAddressBuffer::ByteAddressBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource)
    :
    Buffer(device, resource)
{}
