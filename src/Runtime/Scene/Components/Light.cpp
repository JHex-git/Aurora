// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/Components/Light.h"
#include "Runtime/Scene/LightManager.h"

namespace Aurora
{
REFLECTABLE_IMPL(Light, m_color, glm::vec3)
REFLECTABLE_IMPL(Light, m_intensity, float)

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
    color_node->SetAttribute("R", m_color.r);
    color_node->SetAttribute("G", m_color.g);
    color_node->SetAttribute("B", m_color.b);

    auto intensity_node = node->InsertNewChildElement("Intensity");
    intensity_node->SetAttribute("Value", m_intensity);
}

void Light::Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner)
{
    node->FirstChildElement("Color")->QueryFloatAttribute("R", &m_color.r);
    node->FirstChildElement("Color")->QueryFloatAttribute("G", &m_color.g);
    node->FirstChildElement("Color")->QueryFloatAttribute("B", &m_color.b);

    node->FirstChildElement("Intensity")->QueryFloatAttribute("Value", &m_intensity);

    Init(owner);
}

} // namespace Aurora