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
    enum class Type
    {
        Directional,
        Point,
        Unknown
    };

    static const char* TypeToCString(Type type)
    {
        switch (type)
        {
            case Type::Directional: return "Directional";
            case Type::Point: return "Point";
            default: return "Unknown";
        }
    }

    static Type CStringToType(const char* type)
    {
        if (!strcmp(type,"Directional")) return Type::Directional;
        else if (!strcmp(type, "Point")) return Type::Point;
        else return Type::Unknown;
    }

    Light(glm::vec3 color, Type type) : Component("Light"), m_color(color), m_type(type) { }
    Light() : Component("Light") { }
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
    float GetIntensity() const { return m_intensity; }
    glm::vec3 GetDirection();
    Type GetType() const { return m_type; }

private:

    REFLECTABLE_DECLARE(Light, m_color)
    glm::vec3 m_color = glm::vec3(1.0f);

    REFLECTABLE_DECLARE(Light, m_intensity)
    float m_intensity = 1.0f;
    
    static constexpr LightID INVALID_LIGHT_ID = 0;
    LightID m_light_id = INVALID_LIGHT_ID;

    Type m_type = Type::Point;
};
} // namespace Aurora