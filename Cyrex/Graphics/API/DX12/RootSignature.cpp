#include "RootSignature.h"
#include "DXException.h"
#include "Device.h"
#include <cassert>

namespace wrl = Microsoft::WRL;

Cyrex::RootSignature::RootSignature(Device& device, const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc) 
    :
    m_device(device),
    m_rootSignatureDesc{},
    m_numDescriptorsPerTable{ 0 },
    m_samplerTableBitMask(0),
    m_descriptorTableBitMask(0)
{
    SetRootSignatureDesc(rootSignatureDesc);
}

Cyrex::RootSignature::~RootSignature() {
    Destroy();
}

void Cyrex::RootSignature::Destroy() {
    for (uint32_t i = 0; i < m_rootSignatureDesc.NumParameters; i++) {
        const auto& rootParam = m_rootSignatureDesc.pParameters[i];

        if (rootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
            delete[] rootParam.DescriptorTable.pDescriptorRanges;
        }
    }

    delete[] m_rootSignatureDesc.pParameters;
    m_rootSignatureDesc.pParameters   = nullptr;
    m_rootSignatureDesc.NumParameters = 0;

    delete[] m_rootSignatureDesc.pStaticSamplers;
    m_rootSignatureDesc.pStaticSamplers   = nullptr;
    m_rootSignatureDesc.NumStaticSamplers = 0;

    m_descriptorTableBitMask = 0;
    m_samplerTableBitMask    = 0;

    memset(m_numDescriptorsPerTable.data(), 0, sizeof(m_numDescriptorsPerTable));
}

void Cyrex::RootSignature::SetRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc)
{
    // Make sure any previously allocated root signature description is cleaned 
    // up first.
    Destroy();

    const auto numParams = rootSignatureDesc.NumParameters;
    D3D12_ROOT_PARAMETER1* pParams = numParams > 0 ? new D3D12_ROOT_PARAMETER1[numParams] : nullptr;

    for (uint32_t i = 0; i < numParams; i++) {
        const auto& rootParam = rootSignatureDesc.pParameters[i];
        pParams[i] = rootParam;

        if (rootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
            auto numDescriptorRanges = rootParam.DescriptorTable.NumDescriptorRanges;
            D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges = 
                numDescriptorRanges > 0
                ? new D3D12_DESCRIPTOR_RANGE1[numDescriptorRanges]
                : nullptr;

            memcpy(
                pDescriptorRanges,
                rootParam.DescriptorTable.pDescriptorRanges,
                sizeof(D3D12_DESCRIPTOR_RANGE1) * numDescriptorRanges);

            pParams[i].DescriptorTable.NumDescriptorRanges = numDescriptorRanges;
            pParams[i].DescriptorTable.pDescriptorRanges   = pDescriptorRanges;

            if (numDescriptorRanges > 0) {
                switch (pDescriptorRanges[0].RangeType) {
                case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                    m_descriptorTableBitMask |= (1 << i);
                    break;
                case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                    m_samplerTableBitMask |= (1 << i);
                    break;
                }
            }

            for (uint32_t j = 0; j < numDescriptorRanges; j++) {
                m_numDescriptorsPerTable[i] += pDescriptorRanges[j].NumDescriptors;
            }
        }
    }

    m_rootSignatureDesc.NumParameters = numParams;
    m_rootSignatureDesc.pParameters   = pParams;

    auto numStaticSamplers = rootSignatureDesc.NumStaticSamplers;
    D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = 
        numStaticSamplers > 0 
        ? new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers] 
        : nullptr;

    if (pStaticSamplers) {
        memcpy(
            pStaticSamplers, 
            rootSignatureDesc.pStaticSamplers, 
            sizeof(D3D12_STATIC_SAMPLER_DESC) * numStaticSamplers);
    }

    m_rootSignatureDesc.NumStaticSamplers = numStaticSamplers;
    m_rootSignatureDesc.pStaticSamplers = pStaticSamplers;

    D3D12_ROOT_SIGNATURE_FLAGS flags = rootSignatureDesc.Flags;
    m_rootSignatureDesc.Flags = flags;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionRootSignatureDesc;
    versionRootSignatureDesc.Init_1_1(numParams, pParams, numStaticSamplers, pStaticSamplers, flags);

    D3D_ROOT_SIGNATURE_VERSION highestVersion = m_device.GetHighestRootSignatureVersion();

    // Serialize the root signature.
    wrl::ComPtr<ID3DBlob> rootSignatureBlob;
    wrl::ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(
        &versionRootSignatureDesc,
        highestVersion,
        &rootSignatureBlob,
        &errorBlob));

    const auto d3d12Device = m_device.GetD3D12Device();

    ThrowIfFailed(d3d12Device->CreateRootSignature(
        0,
        rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)));
}

uint32_t Cyrex::RootSignature::GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const {
    uint32_t descriptorTableBitMask = 0;
    
    switch (descriptorHeapType)
    {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        descriptorTableBitMask = m_descriptorTableBitMask;
        break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        descriptorTableBitMask = m_samplerTableBitMask;
        break;
    }

    return descriptorTableBitMask;
}

uint32_t Cyrex::RootSignature::GetNumDescriptors(uint32_t rootIndex) const {
    assert(rootIndex < 32);
    return m_numDescriptorsPerTable.at(rootIndex);
}
