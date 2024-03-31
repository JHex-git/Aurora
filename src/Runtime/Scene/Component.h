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
    Component() = default;
    ~Component() = default;

    virtual bool Init() { return true; }
    virtual void Update() { }

    virtual void* GetThis() override { return this; }

    virtual void Serialize(tinyxml2::XMLElement *node) override = 0;
    virtual void Deserialize(const tinyxml2::XMLElement *node) override = 0;
};

} // namespace Aurora
