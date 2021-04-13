#pragma once
#include "EditorLayer.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <memory>

namespace Cyrex {
    class CommandList;
    class Device;
    class PipelineStateObject;
    class RenderTarget;
    class RootSignature;
    class ShaderResourceView;
    class SwapChain;
    class Texture;

    class D3D12Layer final : public EditorLayer {
    public:
        D3D12Layer(Device& device, SwapChain& swapChain, HWND hWnd) noexcept;
        ~D3D12Layer() = default;

        void Attach() override;
        void Detach() noexcept override;

        void Begin() noexcept override;
        void End(CommandList& cmdList) noexcept override;
    private:
        void CreateRootSignature() noexcept;
        void CreatePSO() noexcept;
        void BuildFontTexture() noexcept;

        ImGuiContext* m_imGuiContext{ nullptr };

        Microsoft::WRL::ComPtr<ID3DBlob> m_vertexShader;
        Microsoft::WRL::ComPtr<ID3DBlob> m_pixelShader;

        std::shared_ptr<Texture> m_fontTexture;
        std::shared_ptr<ShaderResourceView> m_fontSRV;
        std::shared_ptr<RootSignature> m_rootSignature;
        std::shared_ptr<PipelineStateObject> m_pipelineState;

        static constexpr auto m_configFlags = ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
        static constexpr auto m_rootSignatureFlags = 
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS       |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS     |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    };
}
