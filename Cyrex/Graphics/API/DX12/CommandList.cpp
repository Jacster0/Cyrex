#include "CommandList.h"
#include "Core/MemoryHelperFuncs.h"

#include "Buffer.h"
#include "ByteAddressBuffer.h"
#include "CommandQueue.h"
#include "ConstantBuffer.h"
#include "ConstantBufferView.h"
#include "Device.h"
#include "DynamicDescriptorHeap.h"
#include "GenerateMipsPSO.h"
#include "IndexBuffer.h"
#include "PipelineStateObject.h"
#include "RenderTarget.h"
#include "Resource.h"
#include "ResourceStateTracker.h"
#include "RootSignature.h"
#include "ShaderResourceView.h"
#include "StructuredBuffer.h"
#include "Texture.h"
#include "UnorderedAccessView.h"
#include "UploadBuffer.h"
#include "VertexBuffer.h"

#include "Core/Math/Math.h"
#include "DXException.h"
#include <DirectXColors.h>


#include "Extern/DirectXTex/DirectXTex/DirectXTex.h"

#include <xmmintrin.h>
#include <cassert>
#include <filesystem>


namespace wrl = Microsoft::WRL;
namespace fs  = std::filesystem;
namespace dx  = DirectX;

Cyrex::CommandList::CommandList(Device& device, D3D12_COMMAND_LIST_TYPE type)
    :
    m_device(device),
    m_d3d12CommandListType(type)
{
    wrl::ComPtr<ID3D12GraphicsCommandList2> commandList;
    const auto d3d12device = m_device.GetD3D12Device();

    ThrowIfFailed(d3d12device->CreateCommandAllocator(m_d3d12CommandListType, IID_PPV_ARGS(&m_d3d12CommandAllocator)));
    ThrowIfFailed(d3d12device->CreateCommandList(
        0,
        m_d3d12CommandListType,
        m_d3d12CommandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(&commandList)));
    m_d3d12CommandList = commandList;

    m_uploadBuffer         = std::make_unique<UploadBuffer>(device);
    m_resourceStateTracker = std::make_unique<ResourceStateTracker>();

    for (auto i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        m_dynamicDescriptorHeap[i] = std::make_unique<DynamicDescriptorHeap>(device, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
        m_descriptorHeaps[i] = nullptr;
    }
}

Cyrex::CommandList::~CommandList()
{
}

void Cyrex::CommandList::TransitionBarrier(
    const std::shared_ptr<Resource>& resource,
    D3D12_RESOURCE_STATES stateAfter, 
    uint32_t subresource, 
    bool flushBarriers)
{
    if (resource) {
        TransitionBarrier(resource->GetD3D12Resource(), stateAfter, subresource, flushBarriers);
    }
}

void Cyrex::CommandList::TransitionBarrier(
    wrl::ComPtr<ID3D12Resource> resource, 
    D3D12_RESOURCE_STATES stateAfter, 
    uint32_t subresource, 
    bool flushBarriers)
{
    if (resource) {
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            resource.Get(), 
            D3D12_RESOURCE_STATE_COMMON, 
            stateAfter, 
            subresource);

        m_resourceStateTracker->ResourceBarrier(barrier);
    }

    if (flushBarriers) {
        FlushResourceBarriers();
    }
}

void Cyrex::CommandList::UAVBarrier(const std::shared_ptr<Resource>& resource, bool flushBarriers) {
    if (resource) {
        UAVBarrier(resource->GetD3D12Resource(), flushBarriers);
    }
}

void Cyrex::CommandList::UAVBarrier(wrl::ComPtr<ID3D12Resource> resource, bool flushBarriers) {
    auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(resource.Get());

    m_resourceStateTracker->ResourceBarrier(barrier);

    if (flushBarriers) {
        FlushResourceBarriers();
    }
}

void Cyrex::CommandList::AliasingBarrier(
    const std::shared_ptr<Resource>& beforeResource,
    const std::shared_ptr<Resource>& afterResource,
    bool flushBarriers)
{
    auto d3d12BeforeResource = beforeResource ? beforeResource->GetD3D12Resource() : nullptr;
    auto d3d12AfterResource  = afterResource  ? afterResource->GetD3D12Resource()  : nullptr;

    AliasingBarrier(d3d12BeforeResource, d3d12AfterResource, flushBarriers);
}

void Cyrex::CommandList::AliasingBarrier(
    wrl::ComPtr<ID3D12Resource> beforeResource, 
    wrl::ComPtr<ID3D12Resource> afterResource,
    bool flushBarriers)
{
    auto barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(beforeResource.Get(), afterResource.Get());
    m_resourceStateTracker->ResourceBarrier(barrier);

    if (flushBarriers) {
        FlushResourceBarriers();
    }
}

void Cyrex::CommandList::FlushResourceBarriers() {
    m_resourceStateTracker->FlushResourceBarriers(shared_from_this());
}

void Cyrex::CommandList::CopyResource(const std::shared_ptr<Resource>& dstRes, const std::shared_ptr<Resource>& srcRes) {
    CopyResource(dstRes->GetD3D12Resource(), srcRes->GetD3D12Resource());
}

void Cyrex::CommandList::CopyResource(
    wrl::ComPtr<ID3D12Resource> dstRes, 
    Microsoft::WRL::ComPtr<ID3D12Resource> srcRes)
{
    TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_COPY_DEST);
    TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_COPY_SOURCE);

    FlushResourceBarriers();

    m_d3d12CommandList->CopyResource(dstRes.Get(), srcRes.Get());

    TrackResource(dstRes);
    TrackResource(srcRes);
}

std::shared_ptr <Cyrex::VertexBuffer> Cyrex::CommandList::CopyVertexBuffer(
    size_t numVertices, 
    size_t vertexStride, 
    const void* vertexBufferData)
{
    const auto d3d12Resource = CopyBuffer(numVertices * vertexStride, vertexBufferData);

    return m_device.CreateVertexBuffer(d3d12Resource, numVertices, vertexStride);
}

std::shared_ptr <Cyrex::IndexBuffer> Cyrex::CommandList::CopyIndexBuffer(
    size_t numIndicies, 
    DXGI_FORMAT indexFormat, 
    const void* indexBufferData)
{
    size_t elementSize = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
    const auto d3d12Resource = CopyBuffer(numIndicies * elementSize, indexBufferData);

    return m_device.CreateIndexBuffer(d3d12Resource, numIndicies, indexFormat);
}

std::shared_ptr<Cyrex::ConstantBuffer> Cyrex::CommandList::CopyConstantBuffer(size_t bufferSize, const void* bufferData) {
    auto d3d12Resource = CopyBuffer(bufferSize, bufferData);

    return m_device.CreateConstantBuffer(d3d12Resource);
}

std::shared_ptr<Cyrex::ByteAddressBuffer> Cyrex::CommandList::CopyByteAddressBuffer(size_t bufferSize, const void* bufferData) {
    auto d3d12Resource = CopyBuffer(bufferSize, bufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    return m_device.CreateByteAddressBuffer(d3d12Resource);
}

std::shared_ptr<Cyrex::StructuredBuffer> Cyrex::CommandList::CopyStructuredBuffer(
    size_t numElements, 
    size_t elementSize, 
    const void* bufferData)
{
    auto d3d12Resource = CopyBuffer(numElements * elementSize, bufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    return m_device.CreateStructuredBuffer(d3d12Resource, numElements, elementSize);
}

void Cyrex::CommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology) {
    m_d3d12CommandList->IASetPrimitiveTopology(primitiveTopology);
}

void Cyrex::CommandList::GenerateMips(const std::shared_ptr<Texture>& texture) {
    if (!texture) {
        return;
    }

    const auto d3d12Device = m_device.GetD3D12Device();

    if (m_d3d12CommandListType == D3D12_COMMAND_LIST_TYPE_COPY) {
        if (!m_computeCommandList) {
            m_computeCommandList = m_device.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetCommandList();
        }
        m_computeCommandList->GenerateMips(texture);
        return;
    }

    const auto d3d12Resource = texture->GetD3D12Resource();

    if (!d3d12Resource) {
        return;
    }

    auto resourceDesc = d3d12Resource->GetDesc();

    // If the texture only has a single mip level, do nothing.
    if (resourceDesc.MipLevels == 1)
        return;

    if (resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
        resourceDesc.DepthOrArraySize != 1 || resourceDesc.SampleDesc.Count > 1) 
    {
        throw std::exception("GenerateMips is only supported for non-multi-sampled 2D Textures");
    }

    wrl::ComPtr<ID3D12Resource> uavResource = d3d12Resource;
    // Create an alias of the original resource.
    // This is done to perform a GPU copy of resources with different formats.
    // BGR -> RGB texture copies will fail GPU validation unless performed
    // through an alias of the BRG resource in a placed heap.
    wrl::ComPtr<ID3D12Resource> aliasResource;

    if (!texture->CheckUAVSupport() || (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0) {
        auto aliasDesc = resourceDesc;

        // Placed resources can't be render targets or depth-stencil views
        aliasDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        aliasDesc.Flags &= ~(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        // Describe a UAV compatible resource that is used to perform
        // mipmapping of the original texture.
        auto uavDesc = aliasDesc;
        uavDesc.Format = Texture::GetUAVCompatableFormat(resourceDesc.Format);

        std::array resourceDescs = { aliasDesc, uavDesc };

        //Create a heap that is large enough to store a copy of the original resource
        auto allocationInfo = d3d12Device->GetResourceAllocationInfo(0, resourceDescs.size(), resourceDescs.data());

        D3D12_HEAP_DESC heapDesc                 = {};
        heapDesc.SizeInBytes                     = allocationInfo.SizeInBytes;
        heapDesc.Alignment                       = allocationInfo.Alignment;
        heapDesc.Flags                           = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
        heapDesc.Properties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapDesc.Properties.Type                 = D3D12_HEAP_TYPE_DEFAULT;

        wrl::ComPtr<ID3D12Heap> heap;
        ThrowIfFailed(d3d12Device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));

        TrackResource(heap);

        ThrowIfFailed(d3d12Device->CreatePlacedResource(
            heap.Get(),
            0,
            &aliasDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&aliasResource)));

        ResourceStateTracker::AddGlobalResourceState(aliasResource.Get(), D3D12_RESOURCE_STATE_COMMON);

        TrackResource(aliasResource);

        ThrowIfFailed(d3d12Device->CreatePlacedResource(
            heap.Get(), 
            0, 
            &uavDesc, 
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&uavResource)));

        ResourceStateTracker::AddGlobalResourceState(uavResource.Get(), D3D12_RESOURCE_STATE_COMMON);

        TrackResource(uavResource);

        AliasingBarrier(nullptr, aliasResource);
        CopyResource(aliasResource, d3d12Resource);
        AliasingBarrier(aliasResource, uavResource);
    }

    auto uavTexture = m_device.CreateTexture(uavResource);

    GenerateMips_UAV(uavTexture, Texture::IsSRGBFormat(resourceDesc.Format));

    if (aliasResource) {
        AliasingBarrier(uavResource, aliasResource);
        CopyResource(d3d12Resource, aliasResource);
    }
}

void Cyrex::CommandList::CopyTextureSubresource(
    const std::shared_ptr<Texture>& texture, 
    uint32_t firstSubresource, 
    uint32_t numSubresources, 
    D3D12_SUBRESOURCE_DATA* subresourceData)
{
    assert(texture);

    const auto d3d12Device   = m_device.GetD3D12Device();
    auto destinationResource = texture->GetD3D12Resource();

    if (destinationResource) {
        TransitionBarrier(texture, D3D12_RESOURCE_STATE_COPY_DEST);
        FlushResourceBarriers();

        auto requiredSize = GetRequiredIntermediateSize(destinationResource.Get(), firstSubresource, numSubresources);

        wrl::ComPtr<ID3D12Resource> intermediateResource;
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto buffer = CD3DX12_RESOURCE_DESC::Buffer(requiredSize);

        ThrowIfFailed(d3d12Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&intermediateResource)));

        UpdateSubresources(
            m_d3d12CommandList.Get(), 
            destinationResource.Get(), 
            intermediateResource.Get(), 
            0, 
            firstSubresource,
            numSubresources, 
            subresourceData);

        TrackResource(intermediateResource);
        TrackResource(destinationResource);
    }
}

void Cyrex::CommandList::SetGraphicsDynamicConstantBuffer(
    uint32_t rootParameterIndex, 
    size_t sizeInBytes, 
    const void* bufferData)
{
    auto alloc = m_uploadBuffer->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    memcpy(alloc.CPU, bufferData, sizeInBytes);

    m_d3d12CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, alloc.GPU);
}

void Cyrex::CommandList::SetGraphics32BitConstants(
    uint32_t rootParameterIndex, 
    uint32_t numConstants, 
    const void* constants)
{
    m_d3d12CommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
}

void Cyrex::CommandList::SetCompute32BitConstants(
    uint32_t rootParameterIndex, 
    uint32_t numConstants, 
    const void* constants)
{
    m_d3d12CommandList->SetComputeRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
}

void Cyrex::CommandList::SetVertexBuffer(uint32_t slot, const std::shared_ptr<VertexBuffer>& vertexBuffer) {
    SetVertexBuffers(slot, { vertexBuffer });
}

void Cyrex::CommandList::SetVertexBuffers(
    uint32_t startSlot, 
    const std::vector<std::shared_ptr<VertexBuffer>>& vertexBuffers)
{
    std::vector<D3D12_VERTEX_BUFFER_VIEW> views;
    views.reserve(vertexBuffers.size());

    for (auto vertexBuffer : vertexBuffers) {
        if (vertexBuffer) {
            TransitionBarrier(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            TrackResource(vertexBuffer);

            views.push_back(vertexBuffer->GetVertexBufferView());
        }
    }

    m_d3d12CommandList->IASetVertexBuffers(startSlot, views.size(), views.data());
}

void Cyrex::CommandList::SetDynamicVertexBuffer(
    uint32_t slot, 
    size_t numVertices, 
    size_t vertexSize, 
    const void* vertexBufferData)
{
    size_t bufferSize   = numVertices * vertexSize;
    auto heapAllocation = m_uploadBuffer->Allocate(bufferSize, vertexSize);

    std::memcpy(heapAllocation.CPU, vertexBufferData, bufferSize);

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation           = heapAllocation.GPU;
    vertexBufferView.SizeInBytes              = static_cast<uint32_t>(bufferSize);
    vertexBufferView.StrideInBytes            = static_cast<uint32_t>(vertexSize);

    m_d3d12CommandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
}

void Cyrex::CommandList::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
    if (indexBuffer) {
        TransitionBarrier(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
        TrackResource(indexBuffer);

        auto indexBufferView = indexBuffer->GetIndexBufferView();
        m_d3d12CommandList->IASetIndexBuffer(&indexBufferView);
    }
}

void Cyrex::CommandList::SetDynamicIndexBuffer(
    size_t numIndicies, 
    DXGI_FORMAT indexFormat, 
    const void* indexBufferData) 
{
    size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
    size_t bufferSize = numIndicies * indexSizeInBytes;

    auto heapAlloc = m_uploadBuffer->Allocate(bufferSize, indexSizeInBytes);
    memcpy(heapAlloc.CPU, indexBufferData, bufferSize);

    D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
    indexBufferView.BufferLocation = heapAlloc.GPU;
    indexBufferView.SizeInBytes    = bufferSize;
    indexBufferView.Format         = indexFormat;

    m_d3d12CommandList->IASetIndexBuffer(&indexBufferView);
}

void Cyrex::CommandList::SetGraphicsDynamicStructuredBuffer(
    uint32_t slot, size_t numElements, 
    size_t elementSize, 
    const void* bufferData)
{
    size_t bufferSize = numElements * elementSize;
    auto heapAlloc = m_uploadBuffer->Allocate(bufferSize, elementSize);
    memcpy(heapAlloc.CPU, bufferData, bufferSize);

    m_d3d12CommandList->SetGraphicsRootShaderResourceView(slot, heapAlloc.GPU);
}

void Cyrex::CommandList::SetViewport(const D3D12_VIEWPORT& viewport) {
    SetViewports({ viewport });
}

void Cyrex::CommandList::SetViewports(const std::vector<D3D12_VIEWPORT>& viewports) {
    assert(viewports.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);

    m_d3d12CommandList->RSSetViewports(
        static_cast<uint32_t>(viewports.size()),
        viewports.data());
}

void Cyrex::CommandList::SetScissorRect(const D3D12_RECT& scissorRect) {
    SetScissorRects({ scissorRect });
}

void Cyrex::CommandList::SetScissorRects(const std::vector<D3D12_RECT>& scissorRects) {
    assert(scissorRects.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);

    m_d3d12CommandList->RSSetScissorRects(
        static_cast<uint32_t>(scissorRects.size()),
        scissorRects.data());
}

void Cyrex::CommandList::SetPipelineState(const std::shared_ptr<PipelineStateObject>& pipelineState) {
    assert(pipelineState);

    auto d3d12PipeLineState = pipelineState->GetD3D12PipelineState().Get();

    if (m_pipelineState != d3d12PipeLineState) {
        m_pipelineState = d3d12PipeLineState;

        m_d3d12CommandList->SetPipelineState(d3d12PipeLineState);

        TrackResource(d3d12PipeLineState);
    }
}

void Cyrex::CommandList::SetGraphicsRootSignature(const std::shared_ptr<RootSignature>& rootSignature) {
    assert(rootSignature);

    auto d3d12RootSig = rootSignature->GetRootSignature().Get();

    if (m_rootSignature != d3d12RootSig) {
        m_rootSignature = d3d12RootSig;

        for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
            m_dynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
        }

        m_d3d12CommandList->SetGraphicsRootSignature(m_rootSignature);

        TrackResource(m_rootSignature);
    }
}

void Cyrex::CommandList::SetComputeRootSignature(const std::shared_ptr<RootSignature>& rootSignature) {
    assert(rootSignature);

    auto d3d12RootSig = rootSignature->GetRootSignature().Get();

    if (m_rootSignature != d3d12RootSig) {
        m_rootSignature = d3d12RootSig;

        for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
            m_dynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
        }

        m_d3d12CommandList->SetComputeRootSignature(m_rootSignature);

        TrackResource(m_rootSignature);
    }
}

void Cyrex::CommandList::SetConstantBufferView(
    uint32_t rootParameterIndex, 
    const std::shared_ptr<ConstantBuffer>& buffer, 
    D3D12_RESOURCE_STATES stateAfter, 
    size_t bufferOffset)
{
    if (buffer) {
        const auto d3d12Resource = buffer->GetD3D12Resource();
        TransitionBarrier(d3d12Resource, stateAfter);

        m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageInlineCBV(
            rootParameterIndex, 
            d3d12Resource->GetGPUVirtualAddress() + bufferOffset);

        TrackResource(buffer);
    }
}

void Cyrex::CommandList::SetShaderResourceView(
    uint32_t rootParameterIndex,
    const std::shared_ptr<Buffer>& buffer,
    D3D12_RESOURCE_STATES stateAfter,
    size_t bufferOffset)
{
    if (buffer) {
        const auto resource = buffer->GetD3D12Resource();
        TransitionBarrier(resource, stateAfter);

        m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageInlineSRV(
            rootParameterIndex, 
            resource->GetGPUVirtualAddress() + bufferOffset);

        TrackResource(buffer);
   }
}

void Cyrex::CommandList::SetUnorderedAccessView(
    uint32_t rootParameterIndex,
    const std::shared_ptr<Buffer>& buffer,
    D3D12_RESOURCE_STATES stateAfter,
    size_t bufferOffset)
{
    if (buffer) {
        const auto resource = buffer->GetD3D12Resource();

        m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageInlineUAV(
            rootParameterIndex,
            resource->GetGPUVirtualAddress() + bufferOffset);

        TrackResource(buffer);
   }
}

void Cyrex::CommandList::SetConstantBufferView(
    uint32_t rootParameterIndex, 
    uint32_t descriptorOffset, 
    const std::shared_ptr<ConstantBufferView>& cbv, 
    D3D12_RESOURCE_STATES stateAfter)
{
    assert(cbv);

    if (auto constantBuffer = cbv->GetConstantBuffer()) {
        TransitionBarrier(constantBuffer, stateAfter);
        TrackResource(constantBuffer);
    }

    m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
        rootParameterIndex,
        descriptorOffset,
        1,
        cbv->GetDescriptorHandle());
}

void Cyrex::CommandList::SetShaderResourceView(
    uint32_t rootParameterIndex,
    uint32_t descriptorOffset,
    const std::shared_ptr<ShaderResourceView>& srv,
    D3D12_RESOURCE_STATES stateAfter,
    uint32_t firstSubresource,
    uint32_t numSubresources) 
{
    assert(srv);

    if (auto resource = srv->GetResource()) {
        if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
            for (uint32_t i = 0; i < numSubresources; ++i)
            {
                TransitionBarrier(resource, stateAfter, firstSubresource + i);
            }
        }
        else {
            TransitionBarrier(resource, stateAfter);
        }

        TrackResource(resource);
    }

    m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
        rootParameterIndex, 
        descriptorOffset, 
        1, 
        srv->GetDescriptorHandle());
}

void Cyrex::CommandList::SetShaderResourceView(
    uint32_t rootParameterIndex, 
    uint32_t descriptorOffset, 
    const std::shared_ptr<Texture>& texture,
    D3D12_RESOURCE_STATES stateAfter, 
    uint32_t firstSubresource,
    uint32_t numSubresources)
{
    assert(texture);

    if (texture) {
        if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
            for (uint32_t i = 0; i < numSubresources; i++) {
                TransitionBarrier(texture, stateAfter, firstSubresource + i);
            }
        }
        else {
            TransitionBarrier(texture, stateAfter);
        }
        TrackResource(texture);
    }

    m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
        rootParameterIndex,
        descriptorOffset,
        1,
        texture->GetShaderResourceView());
}

void Cyrex::CommandList::SetUnorderedAccessView(
    uint32_t rootParameterIndex, 
    uint32_t descrptorOffset, 
    const std::shared_ptr<UnorderedAccessView>& uav, 
    D3D12_RESOURCE_STATES stateAfter, 
    uint32_t firstSubresource, 
    uint32_t numSubresources)
{
    assert(uav);

    if (auto resource = uav->GetResource()) {
        if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
            for (uint32_t i = 0; i < numSubresources; ++i) {
                TransitionBarrier(resource, stateAfter, firstSubresource + i);
            }
        }
        else {
            TransitionBarrier(resource, stateAfter);
        }
        TrackResource(resource);
    }

    m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
        rootParameterIndex, 
        descrptorOffset, 
        1, 
        uav->GetDescriptorHandle());
}

void Cyrex::CommandList::SetRenderTarget(const RenderTarget& renderTarget) {
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors;
    renderTargetDescriptors.reserve(AttachmentPoint::NumAttachmentPoints);

    const auto& textures = renderTarget.GetTextures();

    // Bind color targets (max of 8 render targets can be bound to the rendering pipeline
    for (int i = 0; i < 8; i++) {
        auto texture = textures[i];

        if (texture) {
            TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
            renderTargetDescriptors.push_back(texture->GetRenderTargetView());

            TrackResource(texture);
        }
    }

    auto depthTexture = renderTarget.GetTexture(AttachmentPoint::DepthStencil);

    CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilDescriptor(D3D12_DEFAULT);

    if (depthTexture) {
        TransitionBarrier(depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        depthStencilDescriptor = depthTexture->GetDepthStencilView();

        TrackResource(depthTexture);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE* pDSV = depthStencilDescriptor.ptr != 0 ? &depthStencilDescriptor : nullptr;
    
    m_d3d12CommandList->OMSetRenderTargets(
        static_cast<uint32_t>(renderTargetDescriptors.size()),
        renderTargetDescriptors.data(),
        false,
        pDSV);
}

void Cyrex::CommandList::Draw(
    uint32_t vertexCount, 
    uint32_t instanceCount, 
    uint32_t startVertex, 
    uint32_t startInstance)
{
    FlushResourceBarriers();

    for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        m_dynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw(*this);
    }

    m_d3d12CommandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
}

void Cyrex::CommandList::DrawIndexed(
    uint32_t indexCount, 
    uint32_t instanceCount, 
    uint32_t startIndex, 
    int32_t baseVertex, 
    uint32_t startInstance)
{
    FlushResourceBarriers();

    for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        m_dynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw(*this);
    }

    m_d3d12CommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
}

void Cyrex::CommandList::Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) {
    FlushResourceBarriers();

    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        m_dynamicDescriptorHeap[i]->CommitStagedDescriptorsForDispatch(*this);
    }

    m_d3d12CommandList->Dispatch(numGroupsX, numGroupsY, numGroupsZ);
}

void Cyrex::CommandList::Close() {
    FlushResourceBarriers();
    m_d3d12CommandList->Close();
}

bool Cyrex::CommandList::Close(const std::shared_ptr<CommandList>& pendingCommandList) {
    FlushResourceBarriers();
    m_d3d12CommandList->Close();

    uint32_t numPendingBarriers = m_resourceStateTracker->FlushPendingResourceBarriers(pendingCommandList);
    m_resourceStateTracker->CommitFinalResourceStates();

    return numPendingBarriers > 0;
}

void Cyrex::CommandList::Reset() {
    ThrowIfFailed(m_d3d12CommandAllocator->Reset());
    ThrowIfFailed(m_d3d12CommandList->Reset(m_d3d12CommandAllocator.Get(), nullptr));

    m_resourceStateTracker->Reset();
    m_uploadBuffer->Reset();

    ReleaseTrackedObjects();

    for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        m_dynamicDescriptorHeap[i]->Reset();
        m_descriptorHeaps[i] = nullptr;
    }

    m_rootSignature = nullptr;
    m_pipelineState = nullptr;
    m_computeCommandList = nullptr;
}

void Cyrex::CommandList::ReleaseTrackedObjects() {
    m_trackedObjects.clear();
}

void Cyrex::CommandList::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap) {
    if (m_descriptorHeaps[heapType] != heap) {
        m_descriptorHeaps[heapType] = heap;
        BindDescriptorHeaps();
    }
}

void Cyrex::CommandList::SetRootSignature(const std::shared_ptr<RootSignature>& rootSignature, RootSignatureCallback rootSignatureCB) {
    assert(rootSignature);

    const auto d3d12RootSignature = rootSignature->GetRootSignature().Get();

    if (m_rootSignature != d3d12RootSignature) {
        for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
            m_dynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
        }

        rootSignatureCB();
        TrackResource(m_rootSignature);
    }
}

void Cyrex::CommandList::BindDescriptorHeaps() {
    uint32_t numDescriptors = 0;
    ID3D12DescriptorHeap* descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

    for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        if (const auto descriptorHeap = m_descriptorHeaps[i]) {
            descriptorHeaps[numDescriptors++] = descriptorHeap;
        }
    }

    m_d3d12CommandList->SetDescriptorHeaps(numDescriptors, descriptorHeaps);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Cyrex::CommandList::CopyBuffer(
    size_t bufferSize, const void* bufferData,
    D3D12_RESOURCE_FLAGS flags)
{
    wrl::ComPtr<ID3D12Resource> d3d12Resource;
    if (bufferSize == 0) {
        //NULL resource
    }
    else {
        const auto d3d12Device = m_device.GetD3D12Device();
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto buffer = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

        ThrowIfFailed(d3d12Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&d3d12Resource)));

        // Add the resource to the global resource state tracker.
        ResourceStateTracker::AddGlobalResourceState(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON);

        if (bufferData) {
            auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto buffer = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
            wrl::ComPtr<ID3D12Resource> uploadResource;
            ThrowIfFailed(d3d12Device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &buffer,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&uploadResource)));

            D3D12_SUBRESOURCE_DATA subResourceData = {};
            subResourceData.pData      = bufferData;
            subResourceData.RowPitch   = bufferSize;
            subResourceData.SlicePitch = subResourceData.RowPitch;

            m_resourceStateTracker->TransitionResource(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
            FlushResourceBarriers();

            UpdateSubresources(
                m_d3d12CommandList.Get(),
                d3d12Resource.Get(),
                uploadResource.Get(),
                0, 0, 1,
                &subResourceData);

            TrackResource(uploadResource);
        }
        TrackResource(d3d12Resource);
    }
    return d3d12Resource;
}

void Cyrex::CommandList::TrackResource(const std::shared_ptr<Resource>& res) {
    TrackResource(res->GetD3D12Resource());
}

void Cyrex::CommandList::GenerateMips_UAV(const std::shared_ptr<Texture>& texture, bool isSRGB) {
    if (!m_generateMipsPSO) {
        m_generateMipsPSO = std::make_unique<GenerateMipsPSO>(m_device);
    }

    SetPipelineState(m_generateMipsPSO->GetPipelineState());
    SetComputeRootSignature(m_generateMipsPSO->GetRootSignature());

    GenerateMipsCB generateMipsCB;
    generateMipsCB.IsSRGB = isSRGB;

    auto resource     = texture->GetD3D12Resource();
    auto resourceDesc = resource->GetDesc();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = isSRGB ? Texture::GetSRGBFormat(resourceDesc.Format) : resourceDesc.Format;
    srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels             = resourceDesc.MipLevels;

    auto srv = m_device.CreateShaderResourceView(texture, &srvDesc);

    for (uint32_t srcMip = 0; srcMip < resourceDesc.MipLevels - 1u;) {
        uint64_t srcWidth  = resourceDesc.Width >> srcMip;
        uint32_t srcHeight = resourceDesc.Height >> srcMip;
        uint32_t dstWidth  = static_cast<uint32_t>(srcWidth >> 1);
        uint32_t dstHeight = srcHeight >> 1;


        // 0b00(0): Both width and height are even
        // 0b01(1): Width is odd, height is even
        // 0b10(2): Width is even, height is odd
        // 0b11(3): Both width and height are odd
        generateMipsCB.SrcDimension = (srcHeight & 1) << 1 | (srcWidth & 1);

        // How many mipmap levels to compute this pass (max 4 mips per pass)
        DWORD mipCount;

        // The number of times we can half the size of the texture and get
        // exactly a 50% reduction in size
        // A 1 bit in the width or height indicates an odd dimension
        _BitScanForward(&mipCount, (dstWidth == 1 ? dstHeight : dstWidth) | (dstHeight == 1 ? dstWidth : dstHeight));

        mipCount = std::min<DWORD>(4, mipCount + 1);
        mipCount = (srcMip + mipCount) >= resourceDesc.MipLevels ? resourceDesc.MipLevels - srcMip - 1 : mipCount;

        //Restrict the dimensions from becoming 0
        dstWidth  = std::max<DWORD>(1, dstWidth);
        dstHeight = std::max<DWORD>(1, dstHeight);

        generateMipsCB.SrcMipLevel  = srcMip;
        generateMipsCB.NumMipLevels = mipCount;
        generateMipsCB.TexelSize.x  = 1.0f / static_cast<float>(dstWidth);
        generateMipsCB.TexelSize.y  = 1.0f / static_cast<float>(dstHeight);

        SetCompute32BitConstants(GenerateMips::GenerateMipsCB, generateMipsCB);
        SetShaderResourceView(GenerateMips::SrcMip, 0, srv, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, srcMip, 1);

        for (uint32_t mip = 0; mip < mipCount; mip++) {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format             = resourceDesc.Format;
            uavDesc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = srcMip + mip + 1;

            auto uav = m_device.CreateUnorderedAccessView(texture, nullptr, &uavDesc);
            SetUnorderedAccessView(GenerateMips::OutMip, mip, uav, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, srcMip + mip + 1, 1);
        }

        if (mipCount < 4) {
            m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
                GenerateMips::OutMip, 
                mipCount, 
                4 - mipCount, 
                m_generateMipsPSO->GetDefaultUAV());
        }

        Dispatch(Math::DivideByMultiple(dstWidth, 8u), Math::DivideByMultiple(dstHeight, 8u));

        UAVBarrier(texture);
        srcMip += mipCount;
    }
}

void Cyrex::CommandList::ResolveSubresource(
    const std::shared_ptr<Resource>& dstResource, 
    const std::shared_ptr<Resource>& srcResource, 
    uint32_t dstSubresource, 
    uint32_t srcSubresource)
{
    assert(dstResource && srcResource);

    TransitionBarrier(dstResource, D3D12_RESOURCE_STATE_RESOLVE_DEST, dstSubresource);
    TransitionBarrier(srcResource, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, srcSubresource);

    FlushResourceBarriers();

    m_d3d12CommandList->ResolveSubresource(
        dstResource->GetD3D12Resource().Get(),
        dstSubresource,
        srcResource->GetD3D12Resource().Get(),
        srcSubresource,
        dstResource->GetD3D12ResourceDesc().Format);

    TrackResource(srcResource);
    TrackResource(dstResource);
}

void Cyrex::CommandList::TrackResource(Microsoft::WRL::ComPtr<ID3D12Object> object) {
    m_trackedObjects.push_back(object);
}
