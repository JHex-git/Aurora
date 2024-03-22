#pragma once
// std include

// thirdparty include

// Aurora include
#include "Utility/Serializable.h"

namespace Aurora
{

class Component : public Serializable
{
public:
    Component() = default;
    ~Component() = default;

    virtual void Serialize(tinyxml2::XMLElement *node) override = 0;
    virtual void Deserialize(const tinyxml2::XMLElement *node) override = 0;

    virtual void Update() = 0;
};
} // namespace Aurora
