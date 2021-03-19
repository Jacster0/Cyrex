#pragma once
#include <array>
#include <DirectXMath.h>

namespace Cyrex {
    struct PointLightProperties {
        DirectX::XMFLOAT4 Position{ 0.0f, 0.0f, 0.0f, 1.0f };
        DirectX::XMFLOAT4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

        float Ambient{};
        float ConstantAttenuation{ 1.0f };
        float LinearAttenuation{ 0.0f };
        float QuadraticAttenuation{ 0.0f };
    };

    struct SpotLightProperties {
        DirectX::XMFLOAT4 Position{ 0.0f, 0.0f, 0.0f, 1.0f };
        DirectX::XMFLOAT4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

        float Direction{};
        float Ambient{};
        float SpotAngle{ DirectX::XM_PIDIV2 };
        float ConstantAttenuation{ 1.0f };
        float LinearAttenuation{ 0.0f };
        float QuadraticAttenuation{ 0.0f };
    };

    struct DirectionalLightProperties {
        DirectX::XMFLOAT4 Direction{ 0.0f,0.0f,1.0f,1.0f };
        DirectX::XMFLOAT4 Color{ 1.0f,1.0f,1.0f,1.0f };

        float Angle{};
        float Ambient{};
    };

    struct LightProperties {
        PointLightProperties mPointLightProperties;
        SpotLightProperties mSpotLightProperties;
        DirectionalLightProperties mDirectionalLightProperties;
    };

    class LightsEditorPanel {
    public:
        LightsEditorPanel() = default;

        void Show() noexcept;
        const LightProperties& GetLightProperties() const noexcept { return m_lightProperties; }
    private:
        void ShowLightsPanel() noexcept;
        void ShowPointLightOptions() noexcept;
        void ShowSpotLightOptions() noexcept;
        void ShowDirectionalLightOptions() noexcept;

        LightProperties m_lightProperties;
        static constexpr float m_sliderWidth = 350.0f;
        bool m_showWindow = true;
    };
}
