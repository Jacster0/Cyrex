#pragma once

#include "Core/Math/Vector4.h"

namespace Cyrex {
    struct PointLight {
        PointLight() {}

        Cyrex::Math::Vector4 WorldSpacePosition{ 0.0f, 0.0f, 0.0f, 1.0f };
        Cyrex::Math::Vector4 ViewSpacePosition{ 0.0f, 0.0f, 0.0f, 1.0f };

        Cyrex::Math::Vector4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

        float Ambient{};
        float ConstantAttenuation{ 1.0f };
        float LinearAttenuation{ 0.0f };
        float QuadraticAttenuation{ 0.0f };
    };

    struct SpotLight {
        SpotLight() {}

        Cyrex::Math::Vector4 WorldSpacePosition{ 0.0f, 0.0f, 0.0f, 1.0f };
        Cyrex::Math::Vector4 ViewSpacePosition{ 0.0f, 0.0f, 0.0f, 1.0f };

        Cyrex::Math::Vector4 WorldSpaceDirection{ 0.0f, 0.0f, 1.0f, 0.0f };
        Cyrex::Math::Vector4 ViewSpaceDirection{ 0.0f, 0.0f, 1.0f, 0.0f };

        Cyrex::Math::Vector4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

        float Ambient{};
        float SpotAngle{ DirectX::XM_PIDIV2 };
        float ConstantAttenuation{ 1.0f };
        float LinearAttenuation{ 0.0f };
        float QuadraticAttenuation{ 0.0f };

        float Padding[3]{};
    };

    struct DirectionalLight {
        DirectionalLight() {}

        Cyrex::Math::Vector4 WorldSpaceDirection{ 0.0f,0.0f,1.0f,0.0f };
        Cyrex::Math::Vector4 ViewSpaceDirection{ 0.0f,0.0f,1.0f,0.0f };

        Cyrex::Math::Vector4 Color{ 1.0f,1.0f,1.0f,1.0f };

        float Ambient{};
        float Padding[3]{};
    };

}
