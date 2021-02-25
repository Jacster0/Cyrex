#pragma once

#include <DirectXMath.h>

struct PointLight {
    PointLight() {}

    DirectX::XMFLOAT4 WorldSpacePosition{ 0.0f, 0.0f, 0.0f, 1.0f };
    DirectX::XMFLOAT4 ViewSpacePosition{ 0.0f, 0.0f, 0.0f, 1.0f };

    DirectX::XMFLOAT4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

    float Ambient{};
    float ConstantAttenuation{ 1.0f };
    float LinearAttenuation{ 0.0f };
    float QuadraticAttenuation{ 0.0f };
};

struct SpotLight {
    SpotLight() {}

    DirectX::XMFLOAT4 WorldSpacePosition{ 0.0f, 0.0f, 0.0f, 1.0f };
    DirectX::XMFLOAT4 ViewSpacePosition{ 0.0f, 0.0f, 0.0f, 1.0f };

    DirectX::XMFLOAT4 WorldSpaceDirection{ 0.0f, 0.0f, 1.0f, 0.0f };
    DirectX::XMFLOAT4 ViewSpaceDirection{ 0.0f, 0.0f, 1.0f, 0.0f };

    DirectX::XMFLOAT4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

    float Ambient{};
    float SpotAngle{ DirectX::XM_PIDIV2 };
    float ConstantAttenuation{ 1.0f };
    float LinearAttenuation{ 0.0f };
    float QuadraticAttenuation{ 0.0f };

    float Padding[3];
};

struct DirectionalLight {
    DirectionalLight() {}

    DirectX::XMFLOAT4 WorldSpaceDirection{ 0.0f,0.0f,1.0f,0.0f };
    DirectX::XMFLOAT4 ViewSpaceDirection{ 0.0f,0.0f,1.0f,0.0f };

    DirectX::XMFLOAT4 Color{ 1.0f,1.0f,1.0f,1.0f };

    float Ambient{};
    float Padding[3];
};
