#pragma once
// std include

// thirdparty include

// Aurora include
#include "Utility/Serializable.h"
#include "Utility/Reflection/Reflectable.h"

namespace Aurora
{

class Component : public Serializable, public Reflectable
{
public:
    Component(std::string&& class_name) { m_class_name = std::move(class_name); }
    ~Component() = default;

    virtual bool Init(std::shared_ptr<SceneObject> owner) 
    {
        m_scene_object = owner;
        return true;
    }
    virtual void Update() { }

    virtual void* GetThis() override { return this; }

    virtual void Serialize(tinyxml2::XMLElement *node) override = 0;
    virtual void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override = 0;

protected:
    std::weak_ptr<SceneObject> m_scene_object;
};

} // namespace Aurora
