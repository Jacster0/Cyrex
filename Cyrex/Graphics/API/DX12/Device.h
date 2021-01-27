#pragma once
#include "DescriptorAllocation.h"
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <memory>

namespace Cyrex {
    class Adapter;
    class ByteAddressBuffer;
    class CommandQueue;
    class CommandList;
    class DescriptorAllocator;
    class ConstantBuffer;
    class ConstantBufferView;
    class IndexBuffer;
    class PipelineStateObject;
    class RenderTarget;
    class Resource;
    class RootSignature;
    class ShaderResourceView;
    class StructuredBuffer;
    class SwapChain;
    class Texture;
    class UnorderedAccessView;
    class VertexBuffer;

    class Device {
    public:
        static void EnableDebugLayer();
        static void ReportLiveObjects();
        static std::shared_ptr<Device> Create(std::shared_ptr<Adapter> adapter = nullptr);

        std::wstring GetDescription() const;
        CommandQueue& GetCommandQueue(D3D12_COMMAND_LIST_TYPE type);

        Microsoft::WRL::ComPtr<ID3D12Device8> GetD3D12Device() const noexcept { return m_d3d12Device; }
        std::shared_ptr<Adapter> GetAdapter() const noexcept { return m_adapter; }

        DXGI_SAMPLE_DESC GetMultisampleQualityLevels(
            DXGI_FORMAT format,
            uint32_t numSamples = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT,
            D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE) const;

        D3D_ROOT_SIGNATURE_VERSION GetHighestRootSignatureVersion() const noexcept {
            return m_highestRootSignatureVersion;
        }
        uint32_t GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const {
            return m_d3d12Device->GetDescriptorHandleIncrementSize(type); 
        }

        std::shared_ptr<RootSignature> CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);
        std::shared_ptr<SwapChain> CreateSwapChain(HWND hWnd, DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM);

        template<class PiplelineStateStream>
        std::shared_ptr<PipelineStateObject> CreatePipelineStateObject(PiplelineStateStream& pipelineStateStream) {
            D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
                sizeof(PiplelineStateStream),
                &pipelineStateStream };

            return DoCreatePipelineStateObject(static_cast<D3D12_PIPELINE_STATE_STREAM_DESC>(pipelineStateStreamDesc));
        }

        std::shared_ptr<PipelineStateObject> DoCreatePipelineStateObject(const D3D12_PIPELINE_STATE_STREAM_DESC& pipelineStateStreamDesc);

        std::shared_ptr<ConstantBuffer> CreateConstantBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource);

        std::shared_ptr<ByteAddressBuffer> CreateByteAddressBuffer(size_t bufferSize);
        std::shared_ptr<ByteAddressBuffer> CreateByteAddressBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource);

        std::shared_ptr<StructuredBuffer> CreateStructuredBuffer(size_t numElements, size_t elementSize);
        std::shared_ptr<StructuredBuffer> CreateStructuredBuffer(
            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            size_t numElements, size_t elementSize);

        std::shared_ptr<Texture> CreateTexture(
            const D3D12_RESOURCE_DESC& resourceDesc,
            const D3D12_CLEAR_VALUE* clearValue = nullptr);

        std::shared_ptr<Texture> CreateTexture(
            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            const D3D12_CLEAR_VALUE* clearValue = nullptr);

        std::shared_ptr<IndexBuffer> CreateIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat);
        std::shared_ptr<IndexBuffer> CreateIndexBuffer(
            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            size_t numIndices,
            DXGI_FORMAT indexFormat);

        std::shared_ptr<VertexBuffer> CreateVertexBuffer(size_t numVertices, size_t vertexStride);
        std::shared_ptr<VertexBuffer> CreateVertexBuffer(
            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            size_t numVertices,
            size_t vertexStride);

        std::shared_ptr<ConstantBufferView> CreateConstantBufferView(
            const std::shared_ptr<ConstantBuffer>& constantBuffer,
            size_t offset = 0);

        std::shared_ptr<ShaderResourceView> CreateShaderResourceView(
                const std::shared_ptr<Resource>& resource,
                const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr);

        std::shared_ptr<UnorderedAccessView> CreateUnorderedAccessView(
            const std::shared_ptr<Resource>& resource,
                const std::shared_ptr<Resource>& counterResource = nullptr,
                const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr);

        void Flush();

        DescriptorAllocation AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors = 1);
        void ReleaseStaleDescriptors();
    protected:
        explicit Device(std::shared_ptr<Adapter> adapter);
        virtual ~Device();
    private:
        Microsoft::WRL::ComPtr<ID3D12Device8> m_d3d12Device;
        std::shared_ptr<Adapter> m_adapter;

        std::unique_ptr<CommandQueue> m_directCommandQueue;
        std::unique_ptr<CommandQueue> m_computeCommandQueue;
        std::unique_ptr<CommandQueue> m_copyCommandQueue;

        std::unique_ptr<DescriptorAllocator> m_descriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

        D3D_ROOT_SIGNATURE_VERSION m_highestRootSignatureVersion;
    };
}