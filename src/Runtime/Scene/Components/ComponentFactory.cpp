// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Runtime/Scene/Components/ComponentFactory.h"
#include "Runtime/Scene/Components/MeshRenderer.h"
#include "Runtime/Scene/Components/SkyboxRenderer.h"
#include "Runtime/Scene/Components/Light.h"

namespace Aurora
{

ComponentFactory::ComponentFactory()
{
    // TODO: Register all the component creation functions here.
    m_component_creators["MeshRendererComponent"] = []() { return std::make_shared<MeshRenderer>(); };
    m_component_creators["SkyboxRendererComponent"] = []() { return std::make_shared<SkyboxRenderer>(); };
    m_component_creators["LightComponent"] = []() { return std::make_shared<Light>(); };
}

std::shared_ptr<Component> ComponentFactory::Create(const std::string& component_name)
{
    auto iter = m_component_creators.find(component_name);
    if (iter != m_component_creators.end())
    {
        return iter->second();
    }

    spdlog::error("Unknown component {}.", component_name);
    return nullptr;
}
} // namespace Aurora