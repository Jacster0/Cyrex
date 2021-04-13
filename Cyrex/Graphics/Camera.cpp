#include "Camera.h"

using namespace Cyrex;
using namespace Cyrex::Math;

Camera::Camera()
    :
    m_translation(Vector4()),
    m_rotation(Quaternion()),
    m_view(Matrix()),
    m_invView(Matrix()),
    m_proj(Matrix()),
    m_invProj(Matrix())
{}

void Camera::SetLookAt(Vector4 eye, Vector4 target, Vector4 up) {
    m_view = Matrix::CreateLookAtLH(eye, target, up);

    m_translation = eye;
    m_rotation    = Matrix::Transpose(m_view).GetRotation();

    m_invViewDirty = true;
    m_viewDirty    = false;
}

Matrix Camera::GetView() const {
    if (m_viewDirty) {
        UpdateViewMatrix();
    }
    return m_view;
}

Matrix Camera::GetInverseView() const {
    if (m_invViewDirty) {
        m_invView      = Matrix::Inverse(m_invView);
        m_invViewDirty = false;
    }
    return m_invView;
}

void Camera::SetProj(float vfov, float aspectRatio, float zNear, float zFar) {
    m_vFov        = vfov;
    m_aspectRatio = aspectRatio;
    m_nearZ       = zNear;
    m_farZ        = zFar;

    m_projDirty = true;
    m_invProjDirty = true;
}

Matrix Camera::GetProj() const {
    if (m_projDirty) {
        UpdateProjectionMatrix();
    }
    return m_proj;
}

Matrix Camera::GetInverseProj() const {
    if (m_invProjDirty) {
        UpdateInverseProjectionMatrix();
    }
    return m_invProj;
}

void Camera::SetFov(float vFov) noexcept {
    if (m_vFov != vFov) {
        m_vFov = vFov;
        m_projDirty    = true;
        m_invProjDirty = true;
    }
}

void Camera::SetTranslation(Vector4 translation) {
    m_translation = translation;
    m_viewDirty   = true;
}

Vector4 Camera::GetTranslation() const {
    return m_translation;
}

void Camera::SetRotation(Quaternion rotation) {
    m_rotation = rotation;
}

Quaternion Camera::GetRotation() const {
    return m_rotation;
}

void Camera::Translate(Vector4 translation, Space space) {
    Matrix mat = Matrix(translation, m_rotation, Vector3());

    switch (space) {
    case Space::Local:
        m_translation += Vector4::Rotate(translation, m_rotation);
        break;
    case Space::World:
        m_translation += translation;
        break;
    }

    m_translation.w = 1.0f;

    m_viewDirty    = true;
    m_invViewDirty = true;
}

void Camera::Rotate(Quaternion quaternion) {
    m_rotation = quaternion * m_rotation;

    m_viewDirty    = true;
    m_invViewDirty = true;
}

void Camera::UpdateViewMatrix() const {
    const auto& translation = Matrix::CreateTranslation(Vector4::Negate(m_translation));
    const auto& rotation    = Matrix::Transpose(Matrix::CreateRotation(m_rotation));

    m_view = translation * rotation;

    m_viewDirty    = false;
    m_invViewDirty = true;
}

void Camera::UpdateInverseViewMatrix() const {
    if (m_viewDirty) {
        UpdateViewMatrix();
    }
    m_invView = Matrix::Inverse(m_view);
    m_invViewDirty = false;
}

void Camera::UpdateProjectionMatrix() const {
    m_proj = Matrix::CreatePerspectiveFieldOfViewLH(Math::ToRadians(m_vFov), m_aspectRatio, m_nearZ, m_farZ);

    m_projDirty    = false;
    m_invProjDirty = true;
}

void Camera::UpdateInverseProjectionMatrix() const {
    if (m_projDirty) {
        UpdateProjectionMatrix();
    }

    m_invProj = Matrix::Inverse(m_proj);
    m_invProjDirty = false;
}
