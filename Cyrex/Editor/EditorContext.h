#pragma once

#include <memory>
#include <type_traits>

namespace Cyrex {
    struct DefaultUIOptions {
        int ProgressbarWindowFlags;
    };

    enum class LightType {PointLight, SpotLight, DirectionalLight};

    class CommandList;
    class EditorLayer;
    class Graphics;
    class LightsEditorPanel;
    class LightProperties;
    class PointLightProperties;
    
    class EditorContext {
    public:
        EditorContext(Graphics& gfx) noexcept;
        void Render(const std::shared_ptr<EditorLayer>& editorLayer, CommandList& cmdList) noexcept;

        template<typename T> 
        [[nodiscard]]
        const T& Get() const noexcept;
    private:
        std::unique_ptr<LightsEditorPanel> m_lightsPanel;
        const LightProperties& m_lightProps;
        Graphics& m_gfx;
    };

    template<typename T>
    inline const T& EditorContext::Get() const noexcept {
        if constexpr (std::is_same_v<T, LightProperties>) {
            return static_cast<const T&>(m_lightProps);
        }
    }
}
