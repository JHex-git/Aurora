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
    RenderMaterial() = default;
    ~RenderMaterial() = default;

    virtual bool Init() override { return true; }
    virtual void Update() override { }

    virtual void Serialize(tinyxml2::XMLElement *node) override = 0;
    virtual void Deserialize(const tinyxml2::XMLElement *node) override = 0;
};
} // namespace Aurora