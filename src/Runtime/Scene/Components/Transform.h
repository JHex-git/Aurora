#pragma once
// std include

// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
#include "thirdparty/opengl/glm/glm/gtc/quaternion.hpp"
// Aurora include
#include "Runtime/Scene/Components/Component.h"
#include "Utility/Reflection/ReflectionRegister.h"

namespace Aurora
{

class Transform : public Component
{
public:
    Transform() : Component("Transform") { }
    Transform(const Transform& other) : Transform()
    {
        m_position = other.m_position;
        m_rotation = other.m_rotation;
        m_scale = other.m_scale;
    }
    ~Transform() = default;

    void Serialize(tinyxml2::XMLElement *node) override final;
    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override final;

    operator glm::mat4() const;

private:
    REFLECTABLE_DECLARE(Transform, m_position)
    glm::vec3 m_position = glm::vec3(0.0f);
    REFLECTABLE_DECLARE(Transform, m_rotation)
    glm::vec3 m_rotation = glm::vec3(0, 0, 0);
    REFLECTABLE_DECLARE(Transform, m_scale)
    glm::vec3 m_scale = glm::vec3(1.0f);
};

} // namespace Aurora