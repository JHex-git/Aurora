#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Utility/Serializable.h"

namespace Aurora
{

class GlobalRenderMaterial : public Serializable
{
public:
    GlobalRenderMaterial(std::string&& global_render_material_class_name) { m_class_name = std::move(global_render_material_class_name); }
    ~GlobalRenderMaterial() = default;

    virtual bool Init();
    virtual void Update();

    virtual void Serialize(tinyxml2::XMLElement *node);
    virtual void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner);

private:
    std::string m_class_name;
};
} // namespace Aurora