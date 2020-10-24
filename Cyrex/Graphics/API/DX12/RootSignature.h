#pragma once

#include "d3dx12.h"
#include <wrl.h>
#include <vector>
#include <array>

namespace Cyrex {
    class RootSignature {
    public:
        RootSignature() noexcept;
        RootSignature(
            Microsoft::WRL::ComPtr<ID3D12Device2> device,
            const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion
        );
        ~RootSignature();
    public:
        void Destroy();
    public:
        Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const {
            return m_rootSignature;
        }

        void SetRootSignatureDesc(
            Microsoft::WRL::ComPtr<ID3D12Device2> device,
            const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion
        );

        const D3D12_ROOT_SIGNATURE_DESC1& GetRootSignatureDesc() const {
            return m_rootSignatureDesc;
        }

        uint32_t GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const;
        uint32_t GetNumDescriptors(uint32_t rootIndex) const;
    private:
        D3D12_ROOT_SIGNATURE_DESC1 m_rootSignatureDesc;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

        std::array<uint32_t, 32> m_numDescriptorsPerTable;
        int32_t m_samplerTableBitMask;
        uint32_t m_descriptorTableBitMask;
    };
}
