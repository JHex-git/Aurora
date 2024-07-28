// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/Components/Light.h"
#include "Runtime/Scene/LightManager.h"

namespace Aurora
{
REFLECTABLE_IMPL(Light, m_color, glm::vec3)

Light::~Light()
{
    if (m_light_id != INVALID_LIGHT_ID)
        LightManager::GetInstance().UnregisterLight(m_light_id);
}

bool Light::Init(std::shared_ptr<SceneObject> owner)
{
    if (!Component::Init(owner)) return false;

    m_light_id = LightManager::GetInstance().RegisterLight(shared_from_this());
    return true;
}

void Light::Serialize(tinyxml2::XMLElement *node)
{
    node->SetName("LightComponent");

    auto color_node = node->InsertNewChildElement("Color");
    color_node->SetAttribute("r", m_color.r);
    color_node->SetAttribute("g", m_color.g);
    color_node->SetAttribute("b", m_color.b);
}

void Light::Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner)
{
    node->FirstChildElement("Color")->QueryFloatAttribute("r", &m_color.r);
    node->FirstChildElement("Color")->QueryFloatAttribute("g", &m_color.g);
    node->FirstChildElement("Color")->QueryFloatAttribute("b", &m_color.b);

    Init(owner);
}

} // namespace Aurora