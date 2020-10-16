#pragma once
#include "DescriptorAllocation.h"
#include "d3dx12.h"
#include <wrl.h>
#include <map>
#include <memory>
#include <mutex>
#include <queue>

namespace Cyrex {
    class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage> {
    public:
        DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
    public:
        D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const;
        bool HasSpace(uint32_t numDescriptors) const;
        uint32_t NumFreeHandles() const;
        DescriptorAllocation Allocate(uint32_t numDescriptors);
        void Free(DescriptorAllocation&& descriptor, uint64_t frameNumber);
        void ReleaseStaleDescriptors(uint64_t frameNumber);
    public:
        uint32_t ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle);
        void AddNewBlock(uint32_t offset, uint32_t numDescriptors);
        void FreeBlock(uint32_t offset, uint32_t numDescriptors);
    private:
        using OffsetType = uint32_t;
        using SizeType = uint32_t;

        struct FreeBlockInfo;
        using FreeListByOffset = std::map<OffsetType, FreeBlockInfo>;

        // A map that lists the free blocks by size.
        // Needs to be a multimap since multiple blocks can have the same size.
        using FreeListBySize = std::multimap<SizeType, FreeListByOffset::iterator>;

        struct FreeBlockInfo {
            FreeBlockInfo(SizeType size)
                :
                Size(size)
            {}
            SizeType Size;
            FreeListBySize::iterator FreeListBySizeIt;
        };

        struct StaleDescriptorInfo {
            StaleDescriptorInfo(OffsetType offset, SizeType size, uint64_t frame)
                : Offset(offset)
                , Size(size)
                , FrameNumber(frame)
            {}

            // The offset within the descriptor heap.
            OffsetType Offset;
            // The number of descriptors
            SizeType Size;
            // The frame number that the descriptor was freed.
            uint64_t FrameNumber;
        };

        using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;

        FreeListByOffset m_freeListByOffset;
        FreeListBySize m_freeListBySize;
        StaleDescriptorQueue m_staleDescriptors;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_d3d12DescriptorHeap;
        D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_baseDescriptor;

        uint32_t m_descriptorHandleIncrementSize;
        uint32_t m_numDescriptorsInHeap;
        uint32_t m_numFreeHandles;

        std::mutex m_allocationMutex;
    };

}
