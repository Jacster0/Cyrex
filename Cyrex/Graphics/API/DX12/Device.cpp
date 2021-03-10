#include "Device.h"

#include "Adapter.h"
#include "ByteAddressBuffer.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "ConstantBuffer.h"
#include "ConstantBufferView.h"
#include "DescriptorAllocator.h"
#include "IndexBuffer.h"
#include "PipeLineStateObject.h"
#include "ResourceStateTracker.h"
#include "RootSignature.h"
#include "ShaderResourceView.h"
#include "StructuredBuffer.h"
#include "Swapchain.h"
#include "Texture.h"
#include "UnorderedAccessView.h"
#include "VertexBuffer.h"

#include "DXException.h"

#include <dxgidebug.h>
#include <cassert>
#include <array>


namespace wrl    = Microsoft::WRL;
namespace crx    = Cyrex;

namespace Context {
    using namespace Cyrex;
    
    class MakeUnorderedAccessView final : public UnorderedAccessView {
    public:
        MakeUnorderedAccessView(
            Device& device, 
            const std::shared_ptr<Resource>& resource,
            const std::shared_ptr<Resource>& counterResource,
            const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav )
            : 
            UnorderedAccessView(device, resource, counterResource, uav)
        {}
        ~MakeUnorderedAccessView() {}
    };

    class MakeShaderResourceView final : public ShaderResourceView {
    public:
        MakeShaderResourceView(
            Device& device, 
            const std::shared_ptr<Resource>& resource,
            const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
            :
            ShaderResourceView(device, resource, srv)
        {}

        ~MakeShaderResourceView() {}
    };

    class MakeConstantBuffer final : public ConstantBuffer {
    public:
        MakeConstantBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource)
            : 
            ConstantBuffer(device, resource)
        {}

        ~MakeConstantBuffer() {}

    };

    class MakeConstantBufferView final : public ConstantBufferView {
    public:
        MakeConstantBufferView(Device& device, const std::shared_ptr<ConstantBuffer>& constantBuffer, size_t offset)
            : 
            ConstantBufferView(device, constantBuffer, offset)
        {}
        ~MakeConstantBufferView() {}
    };

    class MakeTexture final : public Texture {
    public:
        MakeTexture(Device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue)
            : Texture(device, resourceDesc, clearValue)
        {}
        MakeTexture(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue)
            :
            Texture(device, resource, clearValue)
        {}
        ~MakeTexture() {}
    };

    class MakeStructuredBuffer final : public StructuredBuffer {
    public:
        MakeStructuredBuffer(Device& device, size_t numElements, size_t elementSize)
            : 
            StructuredBuffer(device, numElements, elementSize)
        {}
        ~MakeStructuredBuffer() {}
    };

    class MakeVertexBuffer final : public VertexBuffer {
    public:
        MakeVertexBuffer(Device& device, size_t numVertices, size_t vertexStride)
            : 
            VertexBuffer(device, numVertices, vertexStride)
        {}
        MakeVertexBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride)
            :
            VertexBuffer(device, resource, numVertices, vertexStride)
        {}
        ~MakeVertexBuffer() {}
    };

    class MakeIndexBuffer final : public IndexBuffer {
    public:
        MakeIndexBuffer(Device& device, size_t numIndicies, DXGI_FORMAT indexFormat)
            : 
            IndexBuffer(device, numIndicies, indexFormat)
        {}

        MakeIndexBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndicies,
            DXGI_FORMAT indexFormat)
            : 
            IndexBuffer(device, resource, numIndicies, indexFormat)
        {}

        ~MakeIndexBuffer() {}
    };

    class MakeByteAddressBuffer final : public ByteAddressBuffer {
    public:
        MakeByteAddressBuffer(Device& device, const D3D12_RESOURCE_DESC& desc)
            : 
            ByteAddressBuffer(device, desc)
        {}

        MakeByteAddressBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resoruce)
            : 
            ByteAddressBuffer(device, resoruce)
        {}

        ~MakeByteAddressBuffer() {}
    };

    class MakeDevice final : public Device {
    public:
        MakeDevice(std::shared_ptr<Adapter> adapter)
            :
            Device(adapter)
        {}
        ~MakeDevice() {}
    };

    class MakeSwapChain final : public SwapChain {
    public:
        MakeSwapChain(Device& device, HWND hWnd, DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM)
            :
            SwapChain(device, hWnd, backBufferFormat)
        {}
        ~MakeSwapChain() {}
    };

    class MakeDescriptorAllocator final : public DescriptorAllocator {
    public:
        MakeDescriptorAllocator(Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 256)
            : 
            DescriptorAllocator(device, type, numDescriptorsPerHeap)
        {}

        ~MakeDescriptorAllocator() {}
    };

    class MakeRootSignature final : public RootSignature {
    public:
        MakeRootSignature(Device& device, const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc)
            :
            RootSignature(device, rootSignatureDesc)
        {}
        ~MakeRootSignature() {}
    };

    class MakePipelineStateObject final : public PipelineStateObject {
    public:
        MakePipelineStateObject(Device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc)
            :
            PipelineStateObject(device, desc)
        {}
        ~MakePipelineStateObject() {}
    };

    class MakeCommandQueue final : public CommandQueue {
    public:
        MakeCommandQueue(Device& device, D3D12_COMMAND_LIST_TYPE type) 
            :
            CommandQueue(device, type)
        {}
        ~MakeCommandQueue() {}
    };
}

Cyrex::Device::Device(std::shared_ptr<Adapter> adapter)
    :
    m_adapter(adapter)
{
    if (!m_adapter) {
        m_adapter = Adapter::Create();
        assert(m_adapter);
    }

    auto dxgiAdapter = m_adapter->GetDXGIAdapter();

    ThrowIfFailed(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3d12Device)));

    wrl::ComPtr<ID3D12InfoQueue> pInfoQueue;
    if (SUCCEEDED(m_d3d12Device.As(&pInfoQueue))) {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        std::array severities = { D3D12_MESSAGE_SEVERITY_INFO };
        std::array denyIds = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE };

        D3D12_INFO_QUEUE_FILTER newFilter = {};
        newFilter.DenyList.NumSeverities  = severities.size();
        newFilter.DenyList.pSeverityList  = severities.data();
        newFilter.DenyList.NumIDs         = denyIds.size();
        newFilter.DenyList.pIDList        = denyIds.data();

        ThrowIfFailed(pInfoQueue->PushStorageFilter(&newFilter));
    }

    m_directCommandQueue  = std::make_unique<Context::MakeCommandQueue>(*this, D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_computeCommandQueue = std::make_unique<Context::MakeCommandQueue>(*this, D3D12_COMMAND_LIST_TYPE_COMPUTE);
    m_copyCommandQueue    = std::make_unique<Context::MakeCommandQueue>(*this, D3D12_COMMAND_LIST_TYPE_COPY);

    // Create descriptor allocators
    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        m_descriptorAllocators[i] =
            std::make_unique<Context::MakeDescriptorAllocator>(*this, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
    }

    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData;
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(m_d3d12Device->CheckFeatureSupport(
        D3D12_FEATURE_ROOT_SIGNATURE,
        &featureData,
        sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
    m_highestRootSignatureVersion = featureData.HighestVersion;
}

Cyrex::Device::~Device() {}

void Cyrex::Device::EnableDebugLayer() {
    wrl::ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
}

void Cyrex::Device::ReportLiveObjects() {
    IDXGIDebug1* dxgiDebug;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
    dxgiDebug->Release();
}

std::shared_ptr<Cyrex::Device> Cyrex::Device::Create(std::shared_ptr<Adapter> adapter) {
    return std::make_shared<Context::MakeDevice>(adapter);
}

std::shared_ptr<Cyrex::StructuredBuffer> Cyrex::Device::CreateStructuredBuffer(size_t numElements, size_t elementSize) {
    return std::make_shared<Context::MakeStructuredBuffer>(*this, numElements, elementSize);
}

std::shared_ptr<Cyrex::StructuredBuffer> Cyrex::Device::CreateStructuredBuffer(
    Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
    size_t numElements, 
    size_t elementSize)
{
    return std::make_shared<Context::MakeStructuredBuffer>(*this, numElements, elementSize);
}

std::shared_ptr<Cyrex::Texture> Cyrex::Device::CreateTexture(
    const D3D12_RESOURCE_DESC& resourceDesc, 
    const D3D12_CLEAR_VALUE* clearValue)
{
    return std::make_shared<Context::MakeTexture>(*this, resourceDesc, clearValue);
}

std::shared_ptr<Cyrex::Texture> Cyrex::Device::CreateTexture(
    Microsoft::WRL::ComPtr<ID3D12Resource> resource,
    const D3D12_CLEAR_VALUE* clearValue)
{
    return std::make_shared<Context::MakeTexture>(*this, resource, clearValue);
}

std::shared_ptr<Cyrex::IndexBuffer> Cyrex::Device::CreateIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat) {
    return std::make_shared<Context::MakeIndexBuffer>(*this, numIndicies, indexFormat);
}

std::shared_ptr<Cyrex::IndexBuffer> Cyrex::Device::CreateIndexBuffer(
    Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
    size_t numIndices, 
    DXGI_FORMAT indexFormat)
{
    return std::make_shared<Context::MakeIndexBuffer>(*this, resource,numIndices,indexFormat);
}

std::shared_ptr<Cyrex::VertexBuffer> Cyrex::Device::CreateVertexBuffer(size_t numVertices, size_t vertexStride)
{
    return std::make_shared<Context::MakeVertexBuffer>(*this,numVertices,vertexStride);
}

std::shared_ptr<Cyrex::VertexBuffer> Cyrex::Device::CreateVertexBuffer(
    Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
    size_t numVertices, 
    size_t vertexStride)
{
    return std::make_shared<Context::MakeVertexBuffer>(*this, resource, numVertices, vertexStride);
}

std::shared_ptr<Cyrex::ConstantBufferView> Cyrex::Device::CreateConstantBufferView(
    const std::shared_ptr<ConstantBuffer>& constantBuffer, 
    size_t offset)
{
    return std::make_shared<Context::MakeConstantBufferView>(*this, constantBuffer, offset);
}

std::shared_ptr<Cyrex::ShaderResourceView> Cyrex::Device::CreateShaderResourceView(
    const std::shared_ptr<Resource>& resource, 
    const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
{
    return std::make_shared<Context::MakeShaderResourceView>(*this, resource, srv);
}

std::shared_ptr<Cyrex::UnorderedAccessView> Cyrex::Device::CreateUnorderedAccessView(
    const std::shared_ptr<Resource>& resource, 
    const std::shared_ptr<Resource>& counterResource, 
    const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
{
    return std::make_shared<Context::MakeUnorderedAccessView>(*this, resource, counterResource, uav);
}

std::wstring Cyrex::Device::GetDescription() const {
    return m_adapter->GetDescription();
}

Cyrex::CommandQueue& Cyrex::Device::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) {
    CommandQueue* cmdQueue = nullptr;

    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        cmdQueue = m_directCommandQueue.get();
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        cmdQueue = m_computeCommandQueue.get();
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        cmdQueue = m_copyCommandQueue.get();
        break;
    default:
        assert(false && "Invalid command queue type.");
    }

    return *cmdQueue;
}

Cyrex::DescriptorAllocation Cyrex::Device::AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) {
    return m_descriptorAllocators[type]->Allocate(numDescriptors);
}

void Cyrex::Device::ReleaseStaleDescriptors() {
    for (unsigned i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        m_descriptorAllocators[i]->ReleaseStaleDescriptors();
    }
}

std::shared_ptr<Cyrex::RootSignature> Cyrex::Device::CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc) {
    return std::make_shared<Context::MakeRootSignature>(*this, rootSignatureDesc);
}

std::shared_ptr<Cyrex::SwapChain> Cyrex::Device::CreateSwapChain(HWND hWnd, DXGI_FORMAT backBufferFormat) {
    return std::make_shared<Context::MakeSwapChain>(*this, hWnd, backBufferFormat);
}

DXGI_SAMPLE_DESC Cyrex::Device::GetMultisampleQualityLevels(
    DXGI_FORMAT format, 
    uint32_t numSamples, 
    D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags) const
{
    DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels;
    qualityLevels.Format           = format;
    qualityLevels.SampleCount      = 1;
    qualityLevels.Flags            = flags;
    qualityLevels.NumQualityLevels = 0;

    while (qualityLevels.SampleCount <= numSamples &&
        SUCCEEDED(m_d3d12Device->CheckFeatureSupport(
            D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
            &qualityLevels,
            sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))) &&
        qualityLevels.NumQualityLevels > 0) 
    {
        sampleDesc.Count   = qualityLevels.SampleCount;
        sampleDesc.Quality = qualityLevels.NumQualityLevels - 1;

        qualityLevels.SampleCount *= 2;
    }

    return sampleDesc;
}

std::shared_ptr<Cyrex::PipelineStateObject> Cyrex::Device::DoCreatePipelineStateObject(
    const D3D12_PIPELINE_STATE_STREAM_DESC& pipelineStateStreamDesc) {
    return std::make_shared<Context::MakePipelineStateObject>(*this, pipelineStateStreamDesc);
}

std::shared_ptr<Cyrex::ConstantBuffer> Cyrex::Device::CreateConstantBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource) {
    return std::make_shared<Context::MakeConstantBuffer>(*this, resource);
}

std::shared_ptr<Cyrex::ByteAddressBuffer> Cyrex::Device::CreateByteAddressBuffer(size_t bufferSize) {
    return  std::make_shared<Context::MakeByteAddressBuffer>(
        *this, 
        CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));
}

std::shared_ptr<Cyrex::ByteAddressBuffer> Cyrex::Device::CreateByteAddressBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource) {
    return std::make_shared<Context::MakeByteAddressBuffer>(*this, resource);
}

void Cyrex::Device::Flush() {
    m_directCommandQueue->Flush();
    m_computeCommandQueue->Flush();
    m_copyCommandQueue->Flush();
}
