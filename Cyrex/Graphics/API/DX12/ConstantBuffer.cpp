#include "ConstantBuffer.h"
#include "d3dx12.h"

Cyrex::ConstantBuffer::ConstantBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource)
    :
    Buffer(device,resource)
{
    m_sizeInBytes = GetD3D12ResourceDesc().Width;
}

Cyrex::ConstantBuffer::~ConstantBuffer() {}
