#pragma once
#include "ImGui/imgui.h"
#include "Platform/Windows/CrxWindow.h"

#include <memory>
#include <d3d12.h>
#include <wrl/client.h>

namespace Cyrex {
    class CommandList;
    class Device;
    class PipelineStateObject;
    class RenderTarget;
    class RootSignature;
    class ShaderResourceView;
    class Texture;

    struct Options {
        float Fps;
        uint32_t width;
        uint32_t height;
        bool AnimateLights;
        bool ShowOpenFileDialog;
        bool IsLoading;
    };

    class Editor {
    public:
        Editor(Device& device, HWND hWnd, const RenderTarget& renderTarget);
        ~Editor();

        void NewFrame() const noexcept;
        void Render(const std::shared_ptr<CommandList>& commandList, const RenderTarget& renderTarget);
        void Destroy() noexcept;
        void SetScaling(float scale) const noexcept;

        static void SetDarkThemeColors() noexcept;
        [[nodiscard]] static auto GetIO() noexcept { return ImGui::GetIO(); }
    private:
        Device& m_device;
        HWND m_hWnd;
        ImGuiContext* m_imGuiContext;

        Microsoft::WRL::ComPtr<ID3DBlob> m_vertexShader;
        Microsoft::WRL::ComPtr<ID3DBlob> m_pixelShader;

        std::shared_ptr<Texture> m_fontTexture;
        std::shared_ptr<ShaderResourceView> m_fontSRV;
        std::shared_ptr<RootSignature> m_rootSignature;
        std::shared_ptr<PipelineStateObject> m_pipelineState;
    };
}