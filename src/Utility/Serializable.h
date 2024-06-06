#pragma once
// std include
#include <memory>
// thirdparty include
#include "thirdparty/tinyxml2/tinyxml2.h"
// Aurora include
// #include "Runtime/Scene/SceneObject.h"

namespace Aurora
{

class SceneObject;

class Serializable
{
public:
    virtual void Serialize(tinyxml2::XMLElement *node) = 0;
    virtual void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) = 0;
};
} // namespace Aurora