#pragma once
#include "Resource.h"

namespace Cyrex {
    class Buffer : public Resource {
    public:
    protected:
        Buffer(Device& device, const D3D12_RESOURCE_DESC& desc);
        Buffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource);
    };
}
