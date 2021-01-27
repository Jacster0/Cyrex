#include "PipeLineStateObject.h"
#include "Device.h"
#include "DXException.h"

Cyrex::PipelineStateObject::PipelineStateObject(Device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc)
    :
    m_device(device)
{
    const auto d3d12device = device.GetD3D12Device();

    ThrowIfFailed(d3d12device->CreatePipelineState(&desc, IID_PPV_ARGS(&m_d3d12Pipelinestate)));
}
