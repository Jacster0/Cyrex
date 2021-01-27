#pragma once

#include "d3dx12.h"
#include <wrl.h>
#include <vector>
#include <array>

namespace Cyrex {
    class Device;
    class RootSignature {
    public:
        void Destroy();

        Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const {
            return m_rootSignature;
        }

        void SetRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);

        const D3D12_ROOT_SIGNATURE_DESC1& GetRootSignatureDesc() const {
            return m_rootSignatureDesc;
        }

        uint32_t GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const;
        uint32_t GetNumDescriptors(uint32_t rootIndex) const;
    protected:
        RootSignature(Device& device, const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);
        ~RootSignature();
    private:
        Device& m_device;
        D3D12_ROOT_SIGNATURE_DESC1 m_rootSignatureDesc;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

        std::array<uint32_t, 32> m_numDescriptorsPerTable;
        int32_t m_samplerTableBitMask;
        uint32_t m_descriptorTableBitMask;
    };
}
