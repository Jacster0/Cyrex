#include "EditorContext.h"
#include "EditorLayer.h"

#include "Core/Logger.h"

#include "Graphics/API/DX12/CommandList.h"
#include "Graphics/Graphics.h"
#include "LightsEditorPanel.h"

using namespace Cyrex;

EditorContext::EditorContext(Graphics& gfx) noexcept
    :
    m_gfx(gfx),
    m_lightsPanel(std::make_unique<LightsEditorPanel>()),
    m_lightProps(m_lightsPanel->GetLightProperties())

{
}

void EditorContext::Render(const std::shared_ptr<EditorLayer>& editorLayer, CommandList& cmdList) noexcept {
    editorLayer->Begin();

    DefaultUIOptions uiOptions;
    uiOptions.ProgressbarWindowFlags = ImGuiWindowFlags_NoResize   |
                                       ImGuiWindowFlags_NoMove     |
                                       ImGuiWindowFlags_NoCollapse |
                                       ImGuiWindowFlags_NoScrollbar;

    const auto& loadingData = m_gfx.GetLoadingData();

    //Save the screen width and height
    auto [width, height] = m_gfx.GetScreenSize();

    static bool isOpen = false;
    //Most of the frames will probably not be spent loading scenes.
    if (loadingData.IsSceneLoading) [[unlikely]] {
        ImGui::SetNextWindowPos(ImVec2(width / 2.0f, height / 2.0f), 0, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(width / 2.0f, 0));

        if (ImGui::Begin("Loading ", &isOpen, uiOptions.ProgressbarWindowFlags)) [[likely]] {
            ImGui::ProgressBar(loadingData.LoadingProgress);
            ImGui::Text(loadingData.LoadingText.data());
        }

        ImGui::End();
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O", nullptr, !loadingData.IsSceneLoading)) {
                m_gfx.OnOpenFileDialog();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Options")) {
            auto vSync = static_cast<bool>(m_gfx.GetVsync());

            if (ImGui::MenuItem("V-Sync", "V", &vSync)) {
                m_gfx.SetVsync(static_cast<VSync>(vSync));

                const auto val = (vSync) ? "On" : "Off";
                crxlog::info("Toggled Vsync: ", val);
            }

            ImGui::MenuItem("Animate Lights", "Space", &m_gfx.AnimateLights());
            ImGui::EndMenu();
        }

        static constexpr auto BUFFER_SIZE = 256;
        const auto fps = m_gfx.GetFramesPerSecond();

        char buffer[BUFFER_SIZE];
        sprintf_s(buffer, BUFFER_SIZE, "FPS: %.2f (%.2f ms)  ", fps, 1.0f / fps * 1000.0f);

        const auto fpsTextSize = ImGui::CalcTextSize(buffer);
        ImGui::SameLine(static_cast<float>(width) - fpsTextSize.x);
        ImGui::Text(buffer);

        ImGui::EndMainMenuBar();
    }

    m_lightsPanel->Show();

    editorLayer->End(cmdList);
}