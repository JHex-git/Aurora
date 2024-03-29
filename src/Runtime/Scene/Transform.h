#pragma once
// std include

// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
#include "thirdparty/opengl/glm/glm/gtc/quaternion.hpp"
// Aurora include
#include "Runtime/Scene/Component.h"
#include "Utility/Reflection/ReflectionRegister.h"

namespace Aurora
{

class Transform : public Component
{
public:
    Transform() : Component() { m_class_name = "Transform"; }
    ~Transform() = default;

    void Serialize(tinyxml2::XMLElement *node) override final;
    void Deserialize(const tinyxml2::XMLElement *node) override final;

private:
    REFLECTABLE_DECLARE(Transform, m_position)
    glm::vec3 m_position = glm::vec3(0.0f);
    REFLECTABLE_DECLARE(Transform, m_rotation)
    glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    REFLECTABLE_DECLARE(Transform, m_scale)
    glm::vec3 m_scale = glm::vec3(1.0f);
};

} // namespace Aurora