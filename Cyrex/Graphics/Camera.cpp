#include "Camera.h"

namespace dx = DirectX;
using namespace Cyrex;

Camera::Camera() {
    m_data = new AlignedData();
    m_data->Translation = dx::XMVectorZero();
    m_data->Rotation    = dx::XMQuaternionIdentity();
}

Camera::~Camera() {
    delete m_data;
}

void XM_CALLCONV Camera::SetLookAt(DirectX::FXMVECTOR eye, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up) {
    m_data->View = dx::XMMatrixLookAtLH(eye, target, up);

    m_data->Translation = eye;
    m_data->Rotation    = dx::XMQuaternionRotationMatrix(dx::XMMatrixTranspose(m_data->View));

    m_invViewDirty = true;
    m_viewDirty = false;
}

DirectX::XMMATRIX Camera::GetView() const {
    if (m_viewDirty) {
        UpdateViewMatrix();
    }
    return m_data->View;
}

DirectX::XMMATRIX Camera::GetInverseView() const {
    if (m_invViewDirty) {
        m_data->InvView = dx::XMMatrixInverse(nullptr, m_data->InvView);
        m_invViewDirty = false;
    }
    return m_data->InvView;
}

void Camera::SetProj(float vfov, float aspectRatio, float zNear, float zFar) {
    m_vFov = vfov;
    m_aspectRatio = aspectRatio;
    m_nearZ = zNear;
    m_farZ = zFar;

    m_projDirty = true;
    m_invProjDirty = true;
}

DirectX::XMMATRIX Camera::GetProj() const {
    if (m_projDirty) {
        UpdateProjectionMatrix();
    }
    return m_data->Proj;
}

DirectX::XMMATRIX Camera::GetInverseProj() const {
    if (m_invProjDirty) {
        UpdateInverseProjectionMatrix();
    }
    return m_data->InvProj;
}

void Camera::SetFov(float vFov) noexcept {
    if (m_vFov != vFov) {
        m_vFov = vFov;
        m_projDirty    = true;
        m_invProjDirty = true;
    }
}

void XM_CALLCONV Camera::SetTranslation(DirectX::FXMVECTOR translation) {
    m_data->Translation = translation;
    m_viewDirty = true;
}

DirectX::XMVECTOR Camera::GetTranslation() const {
    return m_data->Translation;
}

void XM_CALLCONV Camera::SetRotation(DirectX::FXMVECTOR rotation) {
    m_data->Rotation = rotation;
}

DirectX::XMVECTOR Camera::GetRotation() const {
    return m_data->Rotation;
}

void XM_CALLCONV Camera::Translate(DirectX::FXMVECTOR translation, Space space) {
    using namespace DirectX;

    switch (space) {
    case Space::Local:
        m_data->Translation += XMVector3Rotate(translation, m_data->Rotation);
        break;
    case Space::World:
        m_data->Translation += translation;
        break;
    }

    m_data->Translation = XMVectorSetW(m_data->Translation, 1.0f);

    m_viewDirty    = true;
    m_invViewDirty = true;
}

void Camera::Rotate(DirectX::FXMVECTOR quaternion) {
    m_data->Rotation = dx::XMQuaternionMultiply(quaternion, m_data->Rotation);

    m_viewDirty    = true;
    m_invViewDirty = true;
}

void Camera::UpdateViewMatrix() const {
    using namespace DirectX;

    XMMATRIX rotationMatrix    = XMMatrixTranspose(XMMatrixRotationQuaternion(m_data->Rotation));
    XMMATRIX translationMatrix = XMMatrixTranslationFromVector(-(m_data->Translation));

    m_data->View = translationMatrix * rotationMatrix;

    m_viewDirty    = false;
    m_invViewDirty = true;
}

void Camera::UpdateInverseViewMatrix() const {
    if (m_viewDirty) {
        UpdateViewMatrix();
    }
    m_data->InvView = dx::XMMatrixInverse(nullptr, m_data->View);
    m_invViewDirty = false;
}

void Camera::UpdateProjectionMatrix() const {
    m_data->Proj = dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(m_vFov), m_aspectRatio, m_nearZ, m_farZ);

    m_projDirty    = false;
    m_invProjDirty = true;
}

void Camera::UpdateInverseProjectionMatrix() const {
    if (m_projDirty) {
        UpdateProjectionMatrix();
    }

    m_data->InvProj = dx::XMMatrixInverse(nullptr, m_data->Proj);
    m_invProjDirty = false;
}
