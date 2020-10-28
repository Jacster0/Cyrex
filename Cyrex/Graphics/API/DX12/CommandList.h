#pragma once
#include <d3d12.h>
#include <wrl.h>

#include <map>
#include <memory>
#include <vector>
#include <functional>

namespace Cyrex {
    class DynamicDescriptorHeap;
    class Resource;
    class ResourceStateTracker;
    class RootSignature;
    class UploadBuffer;

    class CommandList {
    public:
        CommandList(D3D12_COMMAND_LIST_TYPE type);
        ~CommandList();
    public:
        D3D12_COMMAND_LIST_TYPE GetCommandListType() const noexcept { return m_d3d12CommandListType; }
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetGraphicsCommandList() const noexcept { return m_d3d12CommandList; }
    public:
        void TransitionBarrier(
            const Resource& resource,
            D3D12_RESOURCE_STATES stateAfter,
            uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            bool flushBarriers = false);
        void TransitionBarrier(
            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            D3D12_RESOURCE_STATES stateAfter,
            uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            bool flushBarriers = false);

        void UAVBarrier(const Resource& resource, bool flushBarriers = false);
        void UAVBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, bool flushBarriers = false);

        void AliasingBarrier(const Resource& beforeResource, const Resource& afterResource, bool flushBarriers = false);
        void AliasingBarrier(
            Microsoft::WRL::ComPtr<ID3D12Resource> beforeResource,
            Microsoft::WRL::ComPtr<ID3D12Resource> afterResource,
            bool flushBarriers = false);

        void FlushResourceBarriers();

        void CopyResource(Resource& dstRes, const Resource& srcRes);
        void CopyResource(Microsoft::WRL::ComPtr<ID3D12Resource> dstRes, Microsoft::WRL::ComPtr<ID3D12Resource> srcRes);

        void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology);
    public:
        void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData);
        template<typename T>
        void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, const T& data) {
            SetGraphicsDynamicConstantBuffer(rootParameterIndex, sizeof(T), &data);
        }

        void SetGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants);
        template<typename T>
        void SetGraphics32BitConstants(uint32_t rootParameterIndex, const T& constants) {
            static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Size of type must be a multiple of 4 bytes");
            SetGraphics32BitConstants(rootParameterIndex, sizeof(T) / sizeof(uint32_t), &constants);
        }

        void SetCompute32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants);
        template<typename T>
        void SetCompute32BitConstants(uint32_t rootParameterIndex, const T& constants) {
            static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Size of type must be a multiple of 4 bytes");
            SetCompute32BitConstants(rootParameterIndex, sizeof(T) / sizeof(uint32_t), &constants);
        }

        void SetViewport(const D3D12_VIEWPORT& viewport);
        void SetViewports(const std::vector<D3D12_VIEWPORT>& viewports);

        void SetScissorRect(const D3D12_RECT& scissorRect);
        void SetScissorRects(const std::vector<D3D12_RECT>& scissorRects);

        void SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState);
        void SetGraphicsRootSignature(const RootSignature& rootSignature);
        void SetComputeRootSignature(const RootSignature& rootSignature);

        void SetShaderResourceView(
            uint32_t rootParameterIndex,
            uint32_t descriptorOffset,
            const Resource& resource,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            UINT firstSubresource = 0,
            UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr
        );

        void SetUnorderedAccessView(
            uint32_t rootParameterIndex,
            uint32_t descrptorOffset,
            const Resource& resource,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            UINT firstSubresource = 0,
            UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr
        );

        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0);
        void DrawIndexed(
            uint32_t indexCount,
            uint32_t instanceCount = 1,
            uint32_t startIndex = 0,
            int32_t baseVertex = 0,
            uint32_t startInstance = 0);

        bool Close(CommandList& pendingCommandList);
        void Close();

        void Reset();
        void ReleaseTrackedObjects();

        void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap);
    private:
        void TrackResource(Microsoft::WRL::ComPtr<ID3D12Object> object);
        void TrackResource(const Resource& res);

        void BindDescriptorHeaps();

        void SetRootSignature(const RootSignature& rootSignature, std::function<void(void)> pred);
    private:
        using TrackedObjects = std::vector <Microsoft::WRL::ComPtr<ID3D12Object>>;

        D3D12_COMMAND_LIST_TYPE m_d3d12CommandListType;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_d3d12CommandList;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_d3d12CommandAllocator;

        ID3D12RootSignature* m_rootSignature;

        std::unique_ptr<UploadBuffer> m_uploadBuffer;
        std::unique_ptr<ResourceStateTracker> m_resourceStateTracker;
        std::unique_ptr<DynamicDescriptorHeap> m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

        ID3D12DescriptorHeap* m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
        TrackedObjects m_trackedObjects;
    };
}
