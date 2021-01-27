#pragma once

#include <DirectXMath.h>

enum class Space {
    Local,
    World
};

class Camera {
public:
    Camera();
    virtual ~Camera();

    void XM_CALLCONV SetLookAt(DirectX::FXMVECTOR eye, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up);
    DirectX::XMMATRIX GetView() const;
    DirectX::XMMATRIX GetInverseView() const;

    void SetProj(float vfov, float aspectRatio, float zNear, float zFar);
    DirectX::XMMATRIX GetProj() const;
    DirectX::XMMATRIX GetInverseProj() const;

    void SetFov(float vFov) noexcept;
    float GetFov() const noexcept { return m_vFov; }

    void XM_CALLCONV SetTranslation(DirectX::FXMVECTOR translation);
    DirectX::XMVECTOR GetTranslation() const;

    void XM_CALLCONV SetRotation(DirectX::FXMVECTOR rotation);
    DirectX::XMVECTOR GetRotation() const;

    void XM_CALLCONV Translate(DirectX::FXMVECTOR translation, Space space = Space::Local);
    void Rotate(DirectX::FXMVECTOR quaternion);
protected:
    virtual void UpdateViewMatrix() const;
    virtual void UpdateInverseViewMatrix() const;
    virtual void UpdateProjectionMatrix() const;
    virtual void UpdateInverseProjectionMatrix() const;

    struct alignas(16)  AlignedData {
        DirectX::XMVECTOR Translation;
        DirectX::XMVECTOR Rotation;

        DirectX::XMMATRIX View;
        DirectX::XMMATRIX InvView;

        DirectX::XMMATRIX Proj;
        DirectX::XMMATRIX InvProj;
    };

    AlignedData* m_data;

    float m_vFov{45};
    float m_aspectRatio{1.0f};
    float m_nearZ{0.1f};
    float m_farZ{100.0f};

    mutable bool m_viewDirty;
    mutable bool m_invViewDirty;

    mutable bool m_projDirty;
    mutable bool m_invProjDirty;
};