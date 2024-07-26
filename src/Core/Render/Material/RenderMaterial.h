#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Runtime/Scene/Components/Component.h"

namespace Aurora
{

class RenderMaterial : public Component
{
public:
    RenderMaterial(std::string&& render_material_class_name) : Component(std::move(render_material_class_name)) { }
    ~RenderMaterial() = default;

    virtual void Update() override { }

    virtual void Serialize(tinyxml2::XMLElement *node) override = 0;
    virtual void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override = 0;
};
} // namespace Aurora