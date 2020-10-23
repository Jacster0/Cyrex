#pragma once

#include "d3dx12.h"
#include <wrl.h>
#include <memory>
#include <queue>
#include <functional>

namespace Cyrex {
    class CommandList;
    class RootSignature;

    class DynamicDescriptorHeap {
    public:
        DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t numDescriptorsPerHeap = 1024);
        ~DynamicDescriptorHeap();
    public:
        void StageDescriptors(
            uint32_t rootParamIndex,
            uint32_t offset,
            uint32_t numDescriptors,
            const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors
        );

        void CommitStagedDescriptors(
            CommandList& commandList, 
            std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);
        void CommitStagedDescriptorsForDraw(CommandList& commandList);
        void CommitStagedDescriptorsForDispatch(CommandList& commandList);

        D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(CommandList& commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);

        void ParseRootSignature(const RootSignature& rootSignature);
        void Reset();
    private:
        // Request a descriptor heap if one is available.
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RequestDescriptorHeap();
        // Create a new descriptor heap of no descriptor heap is available.
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap();

        // Compute the number of stale descriptors that need to be copied
        // to GPU visible descriptor heap.
        uint32_t ComputeStaleDescriptorCount() const noexcept;

        //Max number of descriptor tables per root signature
        static constexpr uint32_t MAX_DESCRIPTOR_TABLES = 32;

        struct DescriptorTableCache {
            DescriptorTableCache()
                : 
                NumDescriptors(0), 
                BaseDescriptor(nullptr)
            {}

            // Reset the table cache.
            void Reset() {
                NumDescriptors = 0;
                BaseDescriptor = nullptr;
            }

            uint32_t NumDescriptors;
            D3D12_CPU_DESCRIPTOR_HANDLE* BaseDescriptor;
        };

    private:
        D3D12_DESCRIPTOR_HEAP_TYPE m_descriptorHeapType;
        uint32_t m_numDescriptorsPerHeap;
        uint32_t m_descriptorHandleIncrementSize;

        std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> m_descriptorHandleCache;
        DescriptorTableCache m_descriptorTableCache [MAX_DESCRIPTOR_TABLES];

        uint32_t m_descriptorTableBitMask;
        uint32_t m_staleDescriptorTableBitMask;

        using DescriptorHeapPool = std::queue<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>;

        DescriptorHeapPool m_descriptorHeapPool;
        DescriptorHeapPool m_availableDescriptorHeaps;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_currentDescriptorHeap;
        CD3DX12_GPU_DESCRIPTOR_HANDLE m_currentGPUDescriptorHandle;
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_currentCPUDescriptorHandle;

        uint32_t m_numFreeHandles;

    };
}
