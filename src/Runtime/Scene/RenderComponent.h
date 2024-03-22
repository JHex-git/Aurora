#pragma once
// std include

// thirdparty include

// Aurora include
#include "Utility/Serializable.h"

namespace Aurora
{

class RenderComponent : public Serializable
{
public:
    RenderComponent() = default;
    ~RenderComponent() = default;

    virtual void Render() = 0;

    virtual void Serialize(tinyxml2::XMLElement *node) override = 0;
    virtual void Deserialize(const tinyxml2::XMLElement *node) override = 0;
};
} // namespace Aurora
