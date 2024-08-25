#pragma once
// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/Components/Component.h"


namespace Aurora
{

using FieldLayoutFunction = std::function<void(const std::string&, std::shared_ptr<Component>)>;
using ComponentLayoutFunction = std::function<void(const std::string& component_name,
                                                    std::shared_ptr<Component> component, 
                                                    const std::map<std::string, FieldLayoutFunction>& default_field_layout)>;
class ComponentUI
{
public:
    virtual ComponentLayoutFunction Layout() = 0;
};
} // namespace Aurora