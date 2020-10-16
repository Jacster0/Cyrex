#pragma once

#include "Core/MemoryHelperFuncs.h"
#include <wrl.h>
#include <d3d12.h>
#include <memory>
#include <deque>

namespace Cyrex {
    class UploadBuffer {
    public:
        struct Allocation {
            void* CPU;
            D3D12_GPU_VIRTUAL_ADDRESS GPU;
        };
    public:
        explicit UploadBuffer(size_t pageSize = Cyrex::_2MB);
    public:
        size_t GetPageSize() const noexcept { return m_pageSize; }
        Allocation Allocate(size_t sizeInBytes, size_t alignment);
        void Reset();
    private:
        struct Page
        {
            Page(size_t sizeInBytes);
            Page(const Page& rhs) = delete;
            Page& operator=(const Page& rhs) = delete;
            ~Page();

            [[nodiscard]]bool HasSpace(size_t sizeInBytes, size_t alignment) const;
            Allocation Allocate(size_t sizeInBytes, size_t alignment);
            void Reset();

        private:

            Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12Resource;

            void* m_cpuPtr;
            D3D12_GPU_VIRTUAL_ADDRESS m_gpuPtr;

            size_t m_pageSize;
            size_t m_offset;
        };

        std::shared_ptr<Page> RequestPage();
    private:
        using PagePool = std::deque<std::shared_ptr<Page>>;
  
        PagePool m_pagePool;
        PagePool m_availablePages;

        std::shared_ptr<Page> m_currentPage;
        size_t m_pageSize;
    };
}
