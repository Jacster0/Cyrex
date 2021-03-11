#pragma once
#include "ImGui/imgui.h"
#include "Platform/Windows/CrxWindow.h"

#include <memory>

namespace Cyrex {
    enum class EditorTheme { Dark };

    class CommandList;
    class Device;
    class SwapChain;

    class EditorLayer {
    public:
        EditorLayer(Device& device, SwapChain& swapChain, HWND hWnd) noexcept;
        virtual ~EditorLayer();

        virtual void Attach() = 0;
        virtual void Detach() = 0;

        virtual void Begin() = 0;
        virtual void End(CommandList& cmdList) = 0;

        void SetScaling(float scale) const noexcept;
        void SetThemeColors(EditorTheme theme) noexcept;
        [[nodiscard]] static auto& GetIO() noexcept { return ImGui::GetIO(); }
    protected:
        Device& m_device;
        SwapChain& m_swapChain;
        HWND m_hWnd;
    };
}