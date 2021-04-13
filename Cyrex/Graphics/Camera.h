#pragma once

#include <DirectXMath.h>
#include "Core/Math/Common.h"

enum class Space {
    Local,
    World
};

namespace Cyrex {
    class Camera {
    public:
        Camera();
        ~Camera() = default;

        void SetLookAt(Math::Vector4 eye, Math::Vector4 target, Math::Vector4 up);
        Math::Matrix GetView() const;
        Math::Matrix GetInverseView() const;

        void SetProj(float vfov, float aspectRatio, float zNear, float zFar);
        Math::Matrix GetProj() const;
        Math::Matrix GetInverseProj() const;

        void SetFov(float vFov) noexcept;
        float GetFov() const noexcept { return m_vFov; }

        void SetTranslation(Math::Vector4 translation);
        Math::Vector4 GetTranslation() const;

        void SetRotation(Math::Quaternion rotation);
        Math::Quaternion GetRotation() const;

        void Translate(Math::Vector4 translation, Space space = Space::Local);
        void Rotate(Math::Quaternion quaternion);
    private:
        void UpdateViewMatrix() const;
        void UpdateInverseViewMatrix() const;
        void UpdateProjectionMatrix() const;
        void UpdateInverseProjectionMatrix() const;

        float m_vFov{ 45 };
        float m_aspectRatio{ 1.0f };
        float m_nearZ{ 0.1f };
        float m_farZ{ 100.0f };

        mutable bool m_viewDirty;
        mutable bool m_invViewDirty;

        mutable bool m_projDirty;
        mutable bool m_invProjDirty;

        Math::Vector4 m_translation;
        Math::Quaternion m_rotation;

        mutable Math::Matrix m_view;
        mutable Math::Matrix m_invView;

        mutable Math::Matrix m_proj;
        mutable Math::Matrix m_invProj;
    };
}
