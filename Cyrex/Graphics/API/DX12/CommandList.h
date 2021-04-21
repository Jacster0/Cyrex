#pragma once
#include "VertexTypes.h"

#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>

#include "Core/Math/Rectangle.h"
#include "Graphics/Viewport.h"

namespace Cyrex {
    class Buffer;
    class ByteAddressBuffer;
    class ConstantBuffer;
    class ConstantBufferView;
    class Device;
    class DynamicDescriptorHeap;
    class GenerateMipsPSO;
    class IndexBuffer;
    class PipelineStateObject;
    class RenderTarget;
    class Resource;
    class ResourceStateTracker;
    class RootSignature;
    class Scene;
    class ShaderResourceView;
    class StructuredBuffer;
    class Texture;
    class UnorderedAccessView;
    class UploadBuffer;
    class VertexBuffer;

    class CommandList : public std::enable_shared_from_this<CommandList>{
    public:
        D3D12_COMMAND_LIST_TYPE GetCommandListType() const noexcept { return m_d3d12CommandListType; }
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetD3D12CommandList() const noexcept { return m_d3d12CommandList; }
        std::shared_ptr<CommandList> GetGenerateMipsCommandList() const noexcept { return m_computeCommandList; }
        Device& GetDevice() const noexcept { return m_device; };

        void TransitionBarrier(
            const std::shared_ptr<Resource>& resource,
            D3D12_RESOURCE_STATES stateAfter,
            uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            bool flushBarriers = false);
        void TransitionBarrier(
            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            D3D12_RESOURCE_STATES stateAfter,
            uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            bool flushBarriers = false);

        void UAVBarrier(const std::shared_ptr<Resource>& resource, bool flushBarriers = false);
        void UAVBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, bool flushBarriers = false);

        void AliasingBarrier(
            const std::shared_ptr<Resource>& beforeResource, 
            const std::shared_ptr<Resource>& afterResource, 
            bool flushBarriers = false);
        void AliasingBarrier(
            Microsoft::WRL::ComPtr<ID3D12Resource> beforeResource,
            Microsoft::WRL::ComPtr<ID3D12Resource> afterResource,
            bool flushBarriers = false);

        void FlushResourceBarriers();

        void CopyResource(const std::shared_ptr<Resource>& dstRes, const std::shared_ptr<Resource>& srcRes);
        void CopyResource(Microsoft::WRL::ComPtr<ID3D12Resource> dstRes, Microsoft::WRL::ComPtr<ID3D12Resource> srcRes);

        std::shared_ptr<VertexBuffer> CopyVertexBuffer(size_t numVertices, size_t vertexStride,
            const void* vertexBufferData);

        template<typename T>
        std::shared_ptr<VertexBuffer> CopyVertexBuffer(const T& vertexBufferData) {
            return CopyVertexBuffer(vertexBufferData.size(), sizeof(T::value_type), vertexBufferData.data());
        }

        std::shared_ptr<IndexBuffer> CopyIndexBuffer(
            size_t numIndicies, 
            DXGI_FORMAT indexFormat,
            const void* indexBufferData);

        template<typename T>
        std::shared_ptr<IndexBuffer> CopyIndexBuffer(const T& indexBufferData) {
            assert(sizeof(T::value_type) == 2 || sizeof(T::value_type) == 4);

            DXGI_FORMAT indexFormat = (sizeof(T::value_type) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
            return CopyIndexBuffer(indexBufferData.size(), indexFormat, indexBufferData.data());
        }

        std::shared_ptr<ConstantBuffer> CopyConstantBuffer(size_t bufferSize, const void* bufferData);

        template<typename T>
        std::shared_ptr<ConstantBuffer> CopyConstantBuffer(const T& data) {
            return CopyConstantBuffer(sizeof(T), &data);
        }

        std::shared_ptr<ByteAddressBuffer> CopyByteAddressBuffer(size_t bufferSize, const void* bufferData);
        template<typename T>
        std::shared_ptr<ByteAddressBuffer> CopyByteAddressBuffer(const T& data) {
            return CopyByteAddressBuffer(sizeof(T), &data);
        }

        std::shared_ptr<StructuredBuffer> CopyStructuredBuffer(
            size_t numElements, 
            size_t elementSize,
            const void* bufferData);

        template<typename T>
        std::shared_ptr<StructuredBuffer> CopyStructuredBuffer(const T& bufferData) {
            return CopyStructuredBuffer(bufferData.size(), sizeof(T::value_type), bufferData.data());
        }

        void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology);

        void GenerateMips(const std::shared_ptr<Texture>& texture);
     
        void CopyTextureSubresource(const std::shared_ptr<Texture>& texture, 
            uint32_t firstSubresource,
            uint32_t numSubresources, 
            D3D12_SUBRESOURCE_DATA* subresourceData);

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

        void SetVertexBuffer(uint32_t slot, const std::shared_ptr<VertexBuffer>& vertexBuff);
        void SetVertexBuffers(uint32_t startSlot, const std::vector<std::shared_ptr<VertexBuffer>>& vertexBuffers);
       
        void SetDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData);
        template<typename T>
        void SetDynamicVertexBuffer(uint32_t slot, const T& vertexBufferData) {
            SetDynamicVertexBuffer(slot, vertexBufferData.size(), sizeof(T::value_type), vertexBufferData.data());
        }

        void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

        void SetDynamicIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData);

        template<typename T>
        void SetDynamicIndexBuffer(const T& indexBufferData)
        {
            static_assert(sizeof(T::value_type) == 2 || sizeof(T::value_type) == 4);

            DXGI_FORMAT indexFormat = (sizeof(T::value_type) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
            SetDynamicIndexBuffer(indexBufferData.size(), indexFormat, indexBufferData.data());
        }

        void SetGraphicsDynamicStructuredBuffer(uint32_t slot, size_t numElements, size_t elementSize,
            const void* bufferData);

        template<typename T>
        void SetGraphicsDynamicStructuredBuffer(uint32_t slot, const T& bufferData)
        {
            SetGraphicsDynamicStructuredBuffer(slot, bufferData.size(), sizeof(T::value_type), bufferData.data());
        }

        void SetViewport(const Cyrex::Viewport& viewport);
        void SetViewports(const Cyrex::Viewport const* viewports, const uint32_t viewportCount);

        void SetScissorRect(const Cyrex::Math::Rectangle& scissorRect) const noexcept;
        void SetScissorRects(const Math::Rectangle const* scissorRects, const uint32_t scissorCount) const noexcept;

        void SetPipelineState(const std::shared_ptr<PipelineStateObject>& pipelineState);

        void SetGraphicsRootSignature(const std::shared_ptr<RootSignature>& rootSignature);
        void SetComputeRootSignature(const std::shared_ptr<RootSignature>& rootSignature);

        void SetConstantBufferView(uint32_t rootParameterIndex, 
            const std::shared_ptr<ConstantBuffer>& buffer,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
            size_t bufferOffset = 0);

        void SetShaderResourceView(
            uint32_t rootParameterIndex, 
            const std::shared_ptr<Buffer>& buffer,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            size_t bufferOffset = 0);
      
        void SetUnorderedAccessView(
            uint32_t rootParameterIndex, 
            const std::shared_ptr<Buffer>& buffer,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            size_t                bufferOffset = 0);

      
        void SetConstantBufferView(
            uint32_t rootParameterIndex, 
            uint32_t descriptorOffset,
            const std::shared_ptr<ConstantBufferView>& cbv,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

       
        void SetShaderResourceView(
            uint32_t rootParameterIndex, 
            uint32_t descriptorOffset,
            const std::shared_ptr<ShaderResourceView>& texture,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            uint32_t firstSubresource = 0,
            uint32_t numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

        void SetShaderResourceView(
            uint32_t rootParameterIndex,
            uint32_t descriptorOffset,
            const std::shared_ptr<Texture>& texture,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            uint32_t firstSubresource = 0,
            uint32_t numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

        void SetUnorderedAccessView(
            uint32_t rootParameterIndex, 
            uint32_t descrptorOffset,
            const std::shared_ptr<UnorderedAccessView>& uav,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            uint32_t firstSubresource = 0,
            uint32_t numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

        void SetRenderTarget(const RenderTarget& renderTarget);

        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0);
        void DrawIndexed(
            uint32_t indexCount,
            uint32_t instanceCount = 1,
            uint32_t startIndex = 0,
            int32_t baseVertex = 0,
            uint32_t startInstance = 0);

        void Dispatch(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1);

        void ResolveSubresource(
            const std::shared_ptr<Resource>& dstResource,
            const std::shared_ptr<Resource>& srcResource,
            uint32_t dstSubresource = 0,
            uint32_t srcSubresource = 0);

        void TrackResource(const std::shared_ptr<Resource>& res);
    protected:
        friend class CommandQueue;
        friend class DynamicDescriptorHeap;
        friend class std::default_delete<CommandList>;

        CommandList(Device& device, D3D12_COMMAND_LIST_TYPE type);
        virtual ~CommandList();

        bool Close(const std::shared_ptr<CommandList>&);
        void Close();

        void Reset();
        void ReleaseTrackedObjects();

        void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap);
    private:
        void GenerateMips_UAV(const std::shared_ptr<Texture>& texture, bool isSRGB);

        using RootSignatureCallback = std::function<void()>;
        void SetRootSignature(const std::shared_ptr<RootSignature>& rootSignature, RootSignatureCallback rootSignatureCB);
        void BindDescriptorHeaps();

        Microsoft::WRL::ComPtr<ID3D12Resource> CopyBuffer(
            size_t bufferSize, const void* bufferData,
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
    private:
        void TrackResource(Microsoft::WRL::ComPtr<ID3D12Object> object);

        using TrackedObjects = std::vector<Microsoft::WRL::ComPtr<ID3D12Object>>;

        Device& m_device;
        D3D12_COMMAND_LIST_TYPE m_d3d12CommandListType;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_d3d12CommandList;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_d3d12CommandAllocator;

        std::shared_ptr<CommandList> m_computeCommandList;

        ID3D12RootSignature* m_rootSignature;
        ID3D12PipelineState* m_pipelineState;

        std::unique_ptr<UploadBuffer> m_uploadBuffer;
        std::unique_ptr<ResourceStateTracker> m_resourceStateTracker;
        std::unique_ptr<DynamicDescriptorHeap> m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

        ID3D12DescriptorHeap* m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

        std::unique_ptr<GenerateMipsPSO> m_generateMipsPSO;

        TrackedObjects m_trackedObjects;
    };
}