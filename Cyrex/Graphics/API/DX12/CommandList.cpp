#include "CommandList.h"
#include "Core/Application.h"
#include "Resource.h"
#include "RootSignature.h"
#include "DXException.h"
#include "UploadBuffer.h"
#include "ResourceStateTracker.h"
#include "DynamicDescriptorHeap.h"
#include <cassert>

namespace wrl = Microsoft::WRL;

Cyrex::CommandList::CommandList(D3D12_COMMAND_LIST_TYPE type)
    :
    m_d3d12CommandListType(type)
{
    wrl::ComPtr<ID3D12GraphicsCommandList2> commandList;
    const auto device = Application::Get().GetDevice();

    ThrowIfFailed(device->CreateCommandAllocator(m_d3d12CommandListType, IID_PPV_ARGS(&m_d3d12CommandAllocator)));
    ThrowIfFailed(device->CreateCommandList(
        0,
        m_d3d12CommandListType,
        m_d3d12CommandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(&commandList)));
    m_d3d12CommandList = commandList;

    m_uploadBuffer         = std::make_unique<UploadBuffer>();
    m_resourceStateTracker = std::make_unique<ResourceStateTracker>();

    for (auto i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        m_dynamicDescriptorHeap[i] = std::make_unique<DynamicDescriptorHeap>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
        m_descriptorHeaps[i] = nullptr;
    }
}

Cyrex::CommandList::~CommandList()
{
}

void Cyrex::CommandList::TransitionBarrier(
    const Resource& resource, 
    D3D12_RESOURCE_STATES stateAfter, 
    uint32_t subresource, 
    bool flushBarriers)
{
    TransitionBarrier(resource.GetD3D12Resource(), stateAfter, subresource, flushBarriers);
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

void Cyrex::CommandList::UAVBarrier(const Resource& resource, bool flushBarriers) {
    UAVBarrier(resource.GetD3D12Resource(), flushBarriers);
}

void Cyrex::CommandList::UAVBarrier(wrl::ComPtr<ID3D12Resource> resource, bool flushBarriers) {
    auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(resource.Get());
    m_resourceStateTracker->ResourceBarrier(barrier);

    if (flushBarriers) {
        FlushResourceBarriers();
    }
}

void Cyrex::CommandList::AliasingBarrier(
    const Resource& beforeResource, 
    const Resource& afterResource, 
    bool flushBarriers)
{
    AliasingBarrier(beforeResource.GetD3D12Resource(), afterResource.GetD3D12Resource(), flushBarriers);
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
    m_resourceStateTracker->FlushResourceBarriers(*this);
}

void Cyrex::CommandList::CopyResource(Resource& dstRes, const Resource& srcRes) {
    CopyResource(dstRes.GetD3D12Resource(), srcRes.GetD3D12Resource());
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

void Cyrex::CommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology) {
    m_d3d12CommandList->IASetPrimitiveTopology(primitiveTopology);
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

void Cyrex::CommandList::SetPipelineState(wrl::ComPtr<ID3D12PipelineState> pipelineState) {
    m_d3d12CommandList->SetPipelineState(pipelineState.Get());

    TrackResource(pipelineState);
}

void Cyrex::CommandList::SetGraphicsRootSignature(const RootSignature& rootSignature) {
    SetRootSignature(rootSignature, [&, this]() { m_d3d12CommandList->SetGraphicsRootSignature(m_rootSignature); });
}

void Cyrex::CommandList::SetComputeRootSignature(const RootSignature& rootSignature) {
    SetRootSignature(rootSignature, [&, this]() { m_d3d12CommandList->SetComputeRootSignature(m_rootSignature); });
}

void Cyrex::CommandList::SetShaderResourceView(
    uint32_t rootParameterIndex, 
    uint32_t descriptorOffset, 
    const Resource& resource, 
    D3D12_RESOURCE_STATES stateAfter, 
    UINT firstSubresource, 
    UINT numSubresources, 
    const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
{
    if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
        for (uint32_t i = 0; i < numSubresources; i++) {
            TransitionBarrier(resource, stateAfter, firstSubresource + i);
        }
    }

    else {
        TransitionBarrier(resource, stateAfter);
    }

    m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
        rootParameterIndex,
        descriptorOffset,
        1,
        resource.GetShaderResourceView(srv));

    TrackResource(resource);
}

void Cyrex::CommandList::SetUnorderedAccessView(
    uint32_t rootParameterIndex, 
    uint32_t descrptorOffset, 
    const Resource& resource, 
    D3D12_RESOURCE_STATES stateAfter, 
    UINT firstSubresource, 
    UINT numSubresources, 
    const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
{
    if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
        for (uint32_t i = 0; i < numSubresources; ++i) {
            TransitionBarrier(resource, stateAfter, firstSubresource + i);
        }
    }
    else {
        TransitionBarrier(resource, stateAfter);
    }

    m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
        rootParameterIndex, 
        descrptorOffset, 
        1, 
        resource.GetUnorderedAccessView(uav));

    TrackResource(resource);
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

void Cyrex::CommandList::Close() {
    FlushResourceBarriers();
    m_d3d12CommandList->Close();
}

bool Cyrex::CommandList::Close(CommandList& pendingCommandList) {
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

void Cyrex::CommandList::TrackResource(const Resource& res) {
    TrackResource(res.GetD3D12Resource());
}

void Cyrex::CommandList::TrackResource(Microsoft::WRL::ComPtr<ID3D12Object> object) {
    m_trackedObjects.push_back(object);
}

void Cyrex::CommandList::SetRootSignature(const RootSignature& rootSignature, std::function<void(void)> pred) {
    auto d3d12RootSignature = rootSignature.GetRootSignature().Get();

    if (m_rootSignature != d3d12RootSignature) {
        m_rootSignature = d3d12RootSignature;

        for (auto i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
            m_dynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
        }

        pred();
        TrackResource(m_rootSignature);
    }
}
