// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/Camera.h"
#include "Math/ViewFrustum.h"

namespace Aurora
{

void MainCamera::Dolly(bool is_forward)
{
    if (is_forward)
        m_camera->m_position += m_camera->GetDirection() * m_forward_speed;
    else
        m_camera->m_position -= m_camera->GetDirection() * m_forward_speed;
}

void MainCamera::Pan(bool is_right)
{
    if (is_right)
        m_camera->m_position += m_camera->GetRight() * m_horizontal_speed;
    else
        m_camera->m_position -= m_camera->GetRight() * m_horizontal_speed;
}

void MainCamera::Tilt(float yoffset)
{
    constexpr float sensitivity = 0.001f;
    m_camera->m_position += m_camera->GetUp() * yoffset * sensitivity;
}

void MainCamera::Rotate(float xoffset, float yoffset)
{
    constexpr float sensitivity = 0.0005f;
    m_camera->SetDirection(m_camera->GetDirection() + m_camera->GetRight() * xoffset * sensitivity + m_camera->GetUp() * yoffset * sensitivity);
}

void MainCamera::AdjustForwardSpeed(float yoffset) 
{
    constexpr float sensitivity = 0.01f;
    float speed = m_forward_speed + yoffset * sensitivity;
    if (speed >= min_forward_speed && speed <= max_forward_speed) m_forward_speed = speed;
}

MainCamera::operator ViewFrustum() const
{
    return ViewFrustum(m_camera->GetFov(), m_camera->GetAspectRatio(), m_camera->GetNearPlane(), m_camera->GetFarPlane());
}

bool MainCamera::Intersect(const AxisAlignedBoundingBox& aabb) const
{
    auto view_aaabb = m_camera->GetViewMatrix() * aabb;
    return Aurora::Intersect(view_aaabb, ViewFrustum(*this));
}
} // namespace Aurora