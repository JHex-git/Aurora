#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Runtime/Scene/Component.h"

namespace Aurora
{

class RenderMaterial : public Component
{
public:
    RenderMaterial() : Component() { m_class_name = "RenderMaterial"; }
    ~RenderMaterial() = default;

    virtual void Update() override { }

    virtual void Serialize(tinyxml2::XMLElement *node) override = 0;
    virtual void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override = 0;
};
} // namespace Aurora