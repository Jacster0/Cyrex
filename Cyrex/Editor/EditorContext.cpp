#include "EditorContext.h"
#include "EditorLayer.h"

#include "Core/Logger.h"

#include "Graphics/API/DX12/CommandList.h"
#include "Graphics/Graphics.h"

using namespace Cyrex;

void EditorContext::Render(Graphics& gfx, const std::shared_ptr<EditorLayer>& editorLayer, CommandList& cmdList) noexcept {
    editorLayer->Begin();

    DefaultUIOptions uiOptions;
    uiOptions.ProgressbarWindowFlags = ImGuiWindowFlags_NoResize   |
                                       ImGuiWindowFlags_NoMove     |
                                       ImGuiWindowFlags_NoCollapse |
                                       ImGuiWindowFlags_NoScrollbar;

    const auto isLoading = gfx.IsLoadingScene();

    //Save the screen width and height
    auto [width, height] = gfx.GetScreenSize();

    //Most of the frames will probably not be spent loading scenes.
    if (isLoading) [[unlikely]] {
        ImGui::SetNextWindowPos(ImVec2(width / 2.0f, height / 2.0f), 0, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(width / 2.0f, 0));

        ImGui::Begin("Loading ", nullptr, uiOptions.ProgressbarWindowFlags);

        ImGui::ProgressBar(gfx.GetLoadingProgress());
        ImGui::Text(gfx.GetLoadingText().data());

        ImGui::End();
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O", nullptr, !isLoading)) {
                gfx.OnOpenFileDialog();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Options")) {
            auto vSync = static_cast<bool>(gfx.GetVsync());

            if (ImGui::MenuItem("V-Sync", "V", &vSync)) {
                gfx.SetVsync(static_cast<VSync>(vSync));

                const auto val = (vSync) ? "On" : "Off";
                crxlog::info("Toggled Vsync: ", val);
            }

            ImGui::MenuItem("Animate Lights", "Space", &gfx.AnimateLights());
            ImGui::EndMenu();
        }

        static constexpr auto BUFFER_SIZE = 256;
        const auto fps = gfx.GetFramesPerSecond();

        char buffer[BUFFER_SIZE];
        sprintf_s(buffer, BUFFER_SIZE, "FPS: %.2f (%.2f ms)  ", fps, 1.0f / fps * 1000.0f);

        const auto fpsTextSize = ImGui::CalcTextSize(buffer);
        ImGui::SameLine(static_cast<float>(width) - fpsTextSize.x);
        ImGui::Text(buffer);

        ImGui::EndMainMenuBar();
    }
    
    editorLayer->End(cmdList);
}