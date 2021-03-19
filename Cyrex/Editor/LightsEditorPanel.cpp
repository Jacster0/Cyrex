#include "LightsEditorPanel.h"
#include "ImGui/imgui.h"
#include "Core/Logger.h"
#include "Graphics/Lights.h"
#include "Core/Math/Math.h"

using namespace Cyrex;

void LightsEditorPanel::Show() noexcept {
    if (m_showWindow) {
        if (!ImGui::Begin("Lights", &m_showWindow)) [[unlikely]] {
            ImGui::End();
        }
        else [[likely]] {
            ShowLightsPanel();
            ImGui::End();
        }
    }
}

void Cyrex::LightsEditorPanel::ShowLightsPanel() noexcept {
    ShowPointLightOptions();
    ShowDirectionalLightOptions();
    ShowSpotLightOptions();
}

void Cyrex::LightsEditorPanel::ShowPointLightOptions() noexcept {
    auto& lightProperties = m_lightProperties.mPointLightProperties;

    if (!ImGui::CollapsingHeader("Point light")) {
        return;
    }
    ImGui::NewLine();

    ImGui::PushItemWidth(300);

    ImGui::Text("Position");
    ImGui::SliderFloat("X ##pointLight", &lightProperties.Position.x, -20.0f, 20.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Y ##pointLight", &lightProperties.Position.y, -10.0f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Z ##pointLight", &lightProperties.Position.z, -10.0f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::Separator();

    ImGui::NewLine();
    ImGui::Text("Light properties");
    ImGui::SliderFloat("Constant Attenuation ##pointLight",  &lightProperties.ConstantAttenuation,  0.0f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Linear Attenuation ##pointLight",    &lightProperties.LinearAttenuation,    0.0f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Quadratic Attenuation ##pointLight", &lightProperties.QuadraticAttenuation, 0.0f, 1.0f,  "%.7f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Ambient ##pointLight",               &lightProperties.Ambient,              0.0f, 1.0f,  "%.3f", ImGuiSliderFlags_AlwaysClamp);

    ImGui::NewLine();
    ImGui::Text("Color");
    ImGui::SameLine();
    ImGui::ColorEdit3("##pointLightColor", std::bit_cast<float*>(&lightProperties.Color), ImGuiColorEditFlags_NoInputs);

    ImGui::PopItemWidth();
}

void Cyrex::LightsEditorPanel::ShowSpotLightOptions() noexcept {
    auto& lightProperties = m_lightProperties.mSpotLightProperties;

    ImGui::Separator();
    if (!ImGui::CollapsingHeader("Spot light")) {
        return;
    }
    ImGui::NewLine();

    ImGui::Text("Position");
    ImGui::SliderFloat("X ##spotLight", &lightProperties.Position.x, -20.0f, 20.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Y ##spotLight", &lightProperties.Position.y, -10.0f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Z ##spotLight", &lightProperties.Position.z, -10.0f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::NewLine();

    ImGui::PushItemWidth(300);
    ImGui::Text("Light properties");
    ImGui::SliderFloat("Direction ##spotLight", &lightProperties.Direction, 0, 360, "%.0f", ImGuiSliderFlags_AlwaysClamp);

    ImGui::SliderFloat("Constant Attenuation ##spotLight",  &lightProperties.ConstantAttenuation,  0.0f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Linear Attenuation ##spotLight",    &lightProperties.LinearAttenuation,    0.0f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Quadratic Attenuation ##spotLight", &lightProperties.QuadraticAttenuation, 0.0f, 1.0f,  "%.7f", ImGuiSliderFlags_AlwaysClamp);

    ImGui::SliderFloat("SpotAngle",            &lightProperties.SpotAngle, 0.0f, 90.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Ambient ##spotLight",  &lightProperties.Ambient,   0.0f, 1.0f,  "%.3f", ImGuiSliderFlags_AlwaysClamp);
  
    ImGui::NewLine();

    ImGui::Text("Color");
    ImGui::SameLine();
    ImGui::ColorEdit3("## spotlightcolor", std::bit_cast<float*>(&m_lightProperties.mSpotLightProperties.Color), ImGuiColorEditFlags_NoInputs);

    ImGui::PopItemWidth();
}

void Cyrex::LightsEditorPanel::ShowDirectionalLightOptions() noexcept {
    auto& lightProperties = m_lightProperties.mDirectionalLightProperties;

    ImGui::Separator();
    if (!ImGui::CollapsingHeader("Directional light")) {
        return;
    }
    ImGui::NewLine();

    ImGui::PushItemWidth(300);
    ImGui::Text("Light properties");
    ImGui::SliderFloat("Direction ##directionalLight", &lightProperties.Angle, 0.0f, 360.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Ambient ##directionalLight", &lightProperties.Ambient, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::Text("Color");
    ImGui::SameLine();
    ImGui::ColorEdit3("##directionalLightColor", std::bit_cast<float*>(&lightProperties.Color), ImGuiColorEditFlags_NoInputs);
    ImGui::PopItemWidth();
}
