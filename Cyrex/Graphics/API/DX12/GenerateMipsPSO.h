#pragma once

#include "DescriptorAllocation.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

namespace Cyrex {
    class Device;
    class PipelineStateObject;
    class RootSignature;

    struct alignas(16) GenerateMipsCB {
        uint32_t SrcMipLevel;
        uint32_t NumMipLevels;
        uint32_t SrcDimension;
        uint32_t IsSRGB; //Must apply gamma correction to sRGB values
        DirectX::XMFLOAT2 TexelSize;
    };

    namespace GenerateMips {
        enum {
            GenerateMipsCB,
            SrcMip,
            OutMip,
            NumRootParameters
        };
    }

    class GenerateMipsPSO {
    public:
        GenerateMipsPSO(Device& device);

        std::shared_ptr<RootSignature> GetRootSignature() const noexcept { return m_rootSignature; }
        std::shared_ptr<PipelineStateObject> GetPipelineState() const noexcept { return m_pipelineState; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultUAV() const { return m_defaultUAV.GetDescriptorHandle(); }
    private:
        std::shared_ptr<RootSignature> m_rootSignature;
        std::shared_ptr<PipelineStateObject> m_pipelineState;

        // If generating less than 4 mip map levels, the unused mip maps
        // need to be padded with default UAVs
        DescriptorAllocation m_defaultUAV;
    };
}