#include "Buffer.h"

Cyrex::Buffer::Buffer(Device& device, const D3D12_RESOURCE_DESC& desc)
    :
    Resource(device, desc)
{}

Cyrex::Buffer::Buffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource)
    :
    Resource(device, resource)
{}
