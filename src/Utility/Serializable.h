#pragma once
// std include

// thirdparty include
#include "thirdparty/tinyxml2/tinyxml2.h"
// Aurora include


namespace Aurora
{

class Serializable
{
public:
    virtual void Serialize(tinyxml2::XMLElement *node) = 0;
    virtual void Deserialize(const tinyxml2::XMLElement *node) = 0;
};
} // namespace Aurora