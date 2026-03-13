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

void MainCamera::MoveVertical(bool is_up)
{
    const float delta = m_forward_speed;
    if (is_up)
        m_camera->m_position += m_world_up * delta;
    else
        m_camera->m_position -= m_world_up * delta;
}

void MainCamera::Tilt(float yoffset)
{
    constexpr float sensitivity = 0.001f;
    m_camera->m_position += m_camera->GetUp() * yoffset * sensitivity;
}

void MainCamera::Rotate(float xoffset, float yoffset)
{
    constexpr float sensitivity = 0.0005f;
    constexpr float pitch_limit = glm::radians(89.0f);

    m_yaw += xoffset * sensitivity;
    m_pitch += yoffset * sensitivity;
    m_pitch = glm::clamp(m_pitch, -pitch_limit, pitch_limit);

    glm::vec3 direction;
    direction.x = std::cos(m_yaw) * std::cos(m_pitch);
    direction.y = std::sin(m_pitch);
    direction.z = std::sin(m_yaw) * std::cos(m_pitch);

    m_camera->SetDirection(direction, m_world_up);
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


