#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "glWrapper/Shader.h"
#include "Runtime/Scene/RenderComponent.h"

namespace Aurora
{

class RenderPass : public RenderComponent
{
public:
    RenderPass() = default;
    ~RenderPass() = default;

    virtual bool Init() { return true; }
    virtual void Render(){}

    virtual void Serialize(tinyxml2::XMLElement *node) override = 0;
    virtual void Deserialize(const tinyxml2::XMLElement *node) override = 0;
};

} // namespace Aurora
