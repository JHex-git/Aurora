#pragma once
// std include

// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
// Aurora include
#include "Utility/Reflection/ReflectionRegister.h"
#include "Runtime/Scene/Components/Component.h"
#include "Runtime/Scene/SceneObjects/SceneObject.h"

namespace Aurora
{
using LightID = unsigned int;

class Light : public Component, public std::enable_shared_from_this<Light>
{
public:
    Light(glm::vec3 color) : Component("Light"), m_color(color) { }
    Light() : Light(glm::vec3(1.0f)) { }
    virtual ~Light();

    virtual void Serialize(tinyxml2::XMLElement *node) override;
    virtual void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override;

    virtual bool Init(std::shared_ptr<SceneObject> owner) override;

    glm::vec3 GetPosition() const 
    { 
        auto scene_object = m_scene_object.lock();
        if (scene_object) return scene_object->GetTransform()->GetField<glm::vec3>("m_position");
        else return glm::vec3();
    }
    glm::vec3 GetColor() const { return m_color; }

private:
    REFLECTABLE_DECLARE(Light, m_color)
    glm::vec3 m_color = glm::vec3(1.0f);
    
    static constexpr LightID INVALID_LIGHT_ID = 0;
    LightID m_light_id = INVALID_LIGHT_ID;
};
} // namespace Aurora