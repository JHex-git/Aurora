#pragma once
// std include

// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
// Aurora include


namespace Aurora
{

class Light
{
public:
    Light(const glm::vec3& color, const glm::vec3& position)
     : m_color(color), m_position(position) { }

    Light(Light&& light) = default;
    Light& operator=(Light&& light) = default;

    Light(const Light&) = default;
    Light& operator=(const Light&) = default;

    ~Light() = default;

    glm::vec3 GetColor() const { return m_color; }
    glm::vec3 GetPosition() const { return m_position; }


private:
    glm::vec3 m_color = glm::vec3(1.0f);
    glm::vec3 m_position = glm::vec3(0.0f);
};

} // namespace Aurora