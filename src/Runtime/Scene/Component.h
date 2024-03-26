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

    virtual bool Init() { return true; }
    virtual void Update() { }

    virtual void Serialize(tinyxml2::XMLElement *node) override = 0;
    virtual void Deserialize(const tinyxml2::XMLElement *node) override = 0;
};
} // namespace Aurora
