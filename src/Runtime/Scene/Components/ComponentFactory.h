#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Runtime/Scene/Components/Component.h"

namespace Aurora
{

class ComponentFactory
{
public:
    static ComponentFactory& GetInstance()
    {
        static ComponentFactory instance;
        return instance;
    }

    ComponentFactory(const ComponentFactory&) = delete;
    ComponentFactory& operator=(const ComponentFactory&) = delete;

    ComponentFactory(ComponentFactory&&) = delete;
    ComponentFactory& operator=(ComponentFactory&&) = delete;

    ~ComponentFactory() = default;

    std::shared_ptr<Component> Create(const std::string& component_name);

private:
    ComponentFactory();

private:
    std::unordered_map<std::string, std::function<std::shared_ptr<Component>()>> m_component_creators;
};
} // namespace Aurora