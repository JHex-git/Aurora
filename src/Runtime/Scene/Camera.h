#pragma once
// std include
#include <memory>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include


namespace Aurora
{

class Camera
{
public:
    Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 up) : m_position(position), m_direction(direction), m_up(up)
    {
        m_direction = glm::normalize(direction);
        m_right = glm::normalize(glm::cross(m_direction, m_up));
        m_up = glm::normalize(glm::cross(m_right, m_direction));
    }

    inline glm::mat4 GetViewMatrix() const { return glm::lookAt(m_position, m_position + m_direction, m_up); }
    inline glm::mat4 GetProjectionMatrix() const { return m_projection_mat; }
    inline glm::vec3 GetDirection() const { return m_direction; }
    inline glm::vec3 GetUp() const { return m_up; }
    inline glm::vec3 GetRight() const { return m_right; }

    inline void SetDirection(glm::vec3 direction) 
    { 
        m_direction = glm::normalize(direction);
        m_right = glm::normalize(glm::cross(m_direction, m_up));
        m_up = glm::normalize(glm::cross(m_right, m_direction));
    }

public:
    glm::vec3 m_position;

protected:
    glm::mat4 m_projection_mat;

private:
    glm::vec3 m_direction;
    glm::vec3 m_up;
    glm::vec3 m_right;
};

class PerspectiveCamera : public Camera
{
public:
    PerspectiveCamera(glm::vec3 position = glm::vec3(0.f, 0.f, 3.f), glm::vec3 direction = glm::vec3(0.f, 0.f, -1.f), glm::vec3 up = glm::vec3(0.f, 1.f, 0.f), float fov = 45.f, float aspect = 800.f / 600.f, float near_plane = 0.1f, float far_plane = 100.f) : Camera(position, direction, up)
    {
        m_projection_mat = glm::perspective(glm::radians(fov), aspect, near_plane, far_plane);
    }
};

class OrthographicCamera : public Camera
{
public:
    OrthographicCamera(glm::vec3 position, glm::vec3 direction, glm::vec3 up, float left, float right, float bottom, float top, float near_plane, float far_plane) : Camera(position, direction, up)
    {
        m_projection_mat = glm::ortho(left, right, bottom, top, near_plane, far_plane);
    }
};

class MainCamera
{
public:
    static MainCamera& GetInstance()
    {
        static MainCamera instance;
        return instance;
    }

    MainCamera(MainCamera const&) = delete;
    void operator=(MainCamera const&) = delete;


    inline glm::mat4 GetViewMatrix() const { return m_camera->GetViewMatrix(); }
    inline glm::mat4 GetProjectionMatrix() const { return m_camera->GetProjectionMatrix(); }
    inline glm::vec3 GetPosition() const { return m_camera->m_position; }

    // 前后移动
    void Dolly(bool is_forward)
    {
        if (is_forward)
            m_camera->m_position += m_camera->GetDirection() * m_forward_speed;
        else
            m_camera->m_position -= m_camera->GetDirection() * m_forward_speed;
    }

    // 左右移动
    void Pan(bool is_right)
    {
        if (is_right)
            m_camera->m_position += m_camera->GetRight() * m_horizontal_speed;
        else
            m_camera->m_position -= m_camera->GetRight() * m_horizontal_speed;
    }

    // 垂直移动
    void Tilt(float yoffset)
    {
        constexpr float sensitivity = 0.001f;
        m_camera->m_position += m_camera->GetUp() * yoffset * sensitivity;
    }

    void Rotate(float xoffset, float yoffset)
    {
        constexpr float sensitivity = 0.0005f;
        m_camera->SetDirection(m_camera->GetDirection() + m_camera->GetRight() * xoffset * sensitivity + m_camera->GetUp() * yoffset * sensitivity);
    }

    void AdjustForwardSpeed(float yoffset) 
    {
        constexpr float sensitivity = 0.01f;
        float speed = m_forward_speed + yoffset * sensitivity;
        if (speed >= min_forward_speed && speed <= max_forward_speed) m_forward_speed = speed;
    }

    float GetForwardSpeed() const { return m_forward_speed; }

private:
    MainCamera()
    {
        m_camera = std::make_unique<PerspectiveCamera>(glm::vec3(0.f, 0.f, 3.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f), 45.f, 800.f / 600.f, 0.1f, 100.f);
    }

    std::unique_ptr<Camera> m_camera;
    float m_forward_speed = default_forward_speed;
    float m_horizontal_speed = 0.01f;

    constexpr static float min_forward_speed = 0.01f;
    constexpr static float max_forward_speed = 5.f;
    constexpr static float default_forward_speed = 0.05f;
};
} // namespace Aurora