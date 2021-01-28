#include "DynamicDescriptorHeap.h"
#include "DXException.h"
#include "RootSignature.h"
#include "Device.h"
#include "CommandList.h"
#include <stdexcept>
#include <cassert>
#include <bitset>

namespace wrl = Microsoft::WRL;

Cyrex::DynamicDescriptorHeap::DynamicDescriptorHeap(
    Device& device,
    D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
    uint32_t numDescriptorsPerHeap)
    :
    m_device(device),
    m_descriptorHeapType(heapType),
    m_numDescriptorsPerHeap(numDescriptorsPerHeap),
    m_descriptorTableBitMask(0),
    m_currentCPUDescriptorHandle(D3D12_DEFAULT),
    m_currentGPUDescriptorHandle(D3D12_DEFAULT),
    m_numFreeHandles(0)
{
    m_descriptorHandleIncrementSize = m_device.GetD3D12Device()->GetDescriptorHandleIncrementSize(heapType);
    m_descriptorHandleCache         = std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(m_numDescriptorsPerHeap);
}

Cyrex::DynamicDescriptorHeap::~DynamicDescriptorHeap()
{
}

void Cyrex::DynamicDescriptorHeap::StageDescriptors(
    uint32_t rootParamIndex, 
    uint32_t offset, 
    uint32_t numDescriptors,
    const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors)
{
   // Cannot stage more than the maximum number of descriptors per heap.
   // Cannot stage more than MaxDescriptorTables root parameters.
    if (numDescriptors > m_numDescriptorsPerHeap || rootParamIndex >= MAX_DESCRIPTOR_TABLES) {
        throw std::bad_alloc();
    }

    DescriptorTableCache& descriptorTableCache = m_descriptorTableCache[rootParamIndex];

    // Check that the number of descriptors to copy does not exceed the number
    // of descriptors expected in the descriptor table.
    if ((offset + numDescriptors) > descriptorTableCache.NumDescriptors) {
        throw std::length_error("Number of descriptors exceeds the number of descriptors in the descriptor table.");
    }

    D3D12_CPU_DESCRIPTOR_HANDLE* dstDescriptor = (descriptorTableCache.BaseDescriptor + offset);
    for (uint32_t i = 0; i < numDescriptors; i++) {
        dstDescriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(srcDescriptors, i, m_descriptorHandleIncrementSize);
    }

    // Set the root parameter index bit to make sure the descriptor table 
    // at that index is bound to the command list.
    m_staleDescriptorTableBitMask |= (1 << rootParamIndex);
}

void Cyrex::DynamicDescriptorHeap::CommitStagedDescriptorsForDraw(CommandList& commandList) {
    CommitStagedDescriptors(commandList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
}

void Cyrex::DynamicDescriptorHeap::CommitStagedDescriptorsForDispatch(CommandList& commandList) {
    CommitStagedDescriptors(commandList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
}

void Cyrex::DynamicDescriptorHeap::CommitStagedDescriptors(
    CommandList& commandList,
    std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc)
{
    auto numDescriptorToCommit = ComputeStaleDescriptorCount();

    if (numDescriptorToCommit > 0) {
        const auto device = m_device.GetD3D12Device();
        const auto d3d12GraphicsCommandList = commandList.GetD3D12CommandList().Get();
        assert(d3d12GraphicsCommandList != nullptr);

        if (!m_currentDescriptorHeap || m_numFreeHandles < numDescriptorToCommit) {
            m_currentDescriptorHeap      = RequestDescriptorHeap();
            m_currentCPUDescriptorHandle = m_currentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            m_currentGPUDescriptorHandle = m_currentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
            m_numFreeHandles             = m_numDescriptorsPerHeap;

            commandList.SetDescriptorHeap(m_descriptorHeapType, m_currentDescriptorHeap.Get());

            m_staleDescriptorTableBitMask = m_descriptorTableBitMask;
        }
        
        constexpr auto numBits = sizeof(m_staleDescriptorTableBitMask) * 8;
        std::bitset<numBits> bitset(m_staleDescriptorTableBitMask);

        //Only enter the loop if any of the bits are set
        if (bitset.any()) [[likely]] {
            for (int i = 0; i < bitset.size(); i++) {
                //if no bits are set we exit the loop
                if (bitset.none()) {
                    break;
                }
                else if (bitset[i]) {
                    uint32_t numDescriptors = m_descriptorTableCache[i].NumDescriptors;
                    D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorHandles = m_descriptorTableCache[i].BaseDescriptor;

                    D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[] = {
                        m_currentCPUDescriptorHandle
                    };

                    uint32_t pDestDescriptorRangeSizes[] = {
                        numDescriptors
                    };

                    device->CopyDescriptors(
                        1,
                        pDestDescriptorRangeStarts,
                        pDestDescriptorRangeSizes,
                        numDescriptors,
                        pSrcDescriptorHandles,
                        nullptr,
                        m_descriptorHeapType);

                    setFunc(d3d12GraphicsCommandList, i, m_currentGPUDescriptorHandle);

                    m_currentCPUDescriptorHandle.Offset(numDescriptors, m_descriptorHandleIncrementSize);
                    m_currentGPUDescriptorHandle.Offset(numDescriptors, m_descriptorHandleIncrementSize);
                    m_numFreeHandles -= numDescriptors;

                    //Flip the bit in order to reduce the number of iterations.
                    bitset[i].flip();
                }
            }
       }
    }
}

D3D12_GPU_DESCRIPTOR_HANDLE Cyrex::DynamicDescriptorHeap::CopyDescriptor(CommandList& commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor) {
    if (!m_currentDescriptorHeap || m_numFreeHandles < 1) {
        m_currentDescriptorHeap      = RequestDescriptorHeap();
        m_currentCPUDescriptorHandle = m_currentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        m_currentGPUDescriptorHandle = m_currentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
        m_numFreeHandles             = m_numDescriptorsPerHeap;

        commandList.SetDescriptorHeap(m_descriptorHeapType, m_currentDescriptorHeap.Get());

        m_staleDescriptorTableBitMask = m_descriptorTableBitMask;
    }

    const auto device = m_device.GetD3D12Device();

    D3D12_GPU_DESCRIPTOR_HANDLE hGpu = m_currentGPUDescriptorHandle;
    device->CopyDescriptorsSimple(1, m_currentCPUDescriptorHandle, cpuDescriptor, m_descriptorHeapType);

    m_currentCPUDescriptorHandle.Offset(1, m_descriptorHandleIncrementSize);
    m_currentGPUDescriptorHandle.Offset(1, m_descriptorHandleIncrementSize);
    m_numFreeHandles -= 1;

    return hGpu;
}

void Cyrex::DynamicDescriptorHeap::StageInlineCBV(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferAddr) {
    assert(rootParameterIndex < MAX_DESCRIPTOR_TABLES);

    m_inlineCBV[rootParameterIndex] = bufferAddr;
    m_staleCBVBitMask |= (1 << rootParameterIndex);
}

void Cyrex::DynamicDescriptorHeap::StageInlineSRV(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferAddr) {
    assert(rootParameterIndex < MAX_DESCRIPTOR_TABLES);

    m_inlineSRV[rootParameterIndex] = bufferAddr;
    m_staleSRVBitMask |= (1 << rootParameterIndex);
}

void Cyrex::DynamicDescriptorHeap::StageInlineUAV(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferAddr) {
    assert(rootParameterIndex < MAX_DESCRIPTOR_TABLES);

    m_inlineUAV[rootParameterIndex] = bufferAddr;
    m_staleUAVBitMask |= (1 << rootParameterIndex);
}

void Cyrex::DynamicDescriptorHeap::ParseRootSignature(const std::shared_ptr<RootSignature>& rootSignature) {
    m_staleDescriptorTableBitMask = 0;
    const auto& rootSignatureDesc = rootSignature->GetRootSignatureDesc();

    // Get a bit mask that represents the root parameter indices that match the 
    // descriptor heap type for this dynamic descriptor heap.
    m_descriptorTableBitMask = rootSignature->GetDescriptorTableBitMask(m_descriptorHeapType);
    uint32_t descriptorTableBitMask = m_descriptorTableBitMask;

    uint32_t currentoffset = 0;

    constexpr auto numBits = sizeof(descriptorTableBitMask) * 8;
    std::bitset<numBits> bitset(descriptorTableBitMask);

    //Only enter the loop if any of the bits are set
    if (bitset.any()) [[likely]] {
        for (int i = 0; i < bitset.size(); i++) {
            //If no bits are set, break the loop
            if (bitset.none()) {
                break;
            }
            else if (bitset[i] && (i < rootSignatureDesc.NumParameters)) {
                uint32_t numDescriptors = rootSignature->GetNumDescriptors(i);

                DescriptorTableCache& descriptorTableCache = m_descriptorTableCache[i];
                descriptorTableCache.NumDescriptors = numDescriptors;
                descriptorTableCache.BaseDescriptor = m_descriptorHandleCache.get() + currentoffset;

                currentoffset += numDescriptors;

                //Flip the bit in order to reduce the number of iterations
                bitset[i].flip();
            }
        }
    }

    assert(currentoffset <= m_numDescriptorsPerHeap &&
        "The root signature requires more than the maximum number of descriptors per descriptor heap."
        "Consider increasing the maximum number of descriptors per descriptor heap."
    );
}

void Cyrex::DynamicDescriptorHeap::Reset() {
    m_availableDescriptorHeaps    = m_descriptorHeapPool;
    m_currentDescriptorHeap.Reset();
    m_currentCPUDescriptorHandle  = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
    m_currentGPUDescriptorHandle  = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
    m_numFreeHandles              = 0;
    m_descriptorTableBitMask      = 0;
    m_staleDescriptorTableBitMask = 0;

    for (uint32_t i = 0; i < MAX_DESCRIPTOR_TABLES; i++) {
        m_descriptorTableCache[i].Reset();
    }
}

wrl::ComPtr<ID3D12DescriptorHeap> Cyrex::DynamicDescriptorHeap::RequestDescriptorHeap() {
    wrl::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    if (!m_availableDescriptorHeaps.empty()) {
        descriptorHeap = m_availableDescriptorHeaps.front();
        m_availableDescriptorHeaps.pop();
    }
    else {
        descriptorHeap = CreateDescriptorHeap();
        m_descriptorHeapPool.push(descriptorHeap);
    }

    return descriptorHeap;
}

wrl::ComPtr<ID3D12DescriptorHeap> Cyrex::DynamicDescriptorHeap::CreateDescriptorHeap() {
    const auto device = m_device.GetD3D12Device();

    D3D12_DESCRIPTOR_HEAP_DESC desciptorHeapDesk = {};
    desciptorHeapDesk.Type           = m_descriptorHeapType;
    desciptorHeapDesk.NumDescriptors = m_numDescriptorsPerHeap;
    desciptorHeapDesk.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    
    wrl::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    ThrowIfFailed(device->CreateDescriptorHeap(&desciptorHeapDesk, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

uint32_t Cyrex::DynamicDescriptorHeap::ComputeStaleDescriptorCount() const noexcept {
    uint32_t numStaleDescriptors = 0;
    const auto staleDescriptorBitMask = m_staleDescriptorTableBitMask;

    constexpr auto numBits = sizeof(staleDescriptorBitMask) * 8;
    std::bitset<numBits> bitset(staleDescriptorBitMask);

    //only enter the loop if any of the bits are set 
    if (bitset.any()) [[likely]] {
        for (int i = 0; i < bitset.size(); i++) {
            //if no bits are set, break the loop.
            if (bitset.none()) {
                break;
            }
            else if (bitset[i]) {
                numStaleDescriptors += m_descriptorTableCache[i].NumDescriptors;

                //Flip the bit in order to reduce the number of iterations
                bitset[i].flip();
            }
        }
    }

    return numStaleDescriptors;
}
