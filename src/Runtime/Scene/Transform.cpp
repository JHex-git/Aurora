// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/Transform.h"

namespace Aurora
{
REFLECTABLE_IMPL(Transform, m_position, glm::vec3)
REFLECTABLE_IMPL(Transform, m_rotation, glm::quat)
REFLECTABLE_IMPL(Transform, m_scale, glm::vec3)

void Transform::Serialize(tinyxml2::XMLElement *node)
{
    node->SetName("TransformComponent");
    auto position_node = node->InsertNewChildElement("Position");
    position_node->SetAttribute("X", m_position.x);
    position_node->SetAttribute("Y", m_position.y);
    position_node->SetAttribute("Z", m_position.z);

    auto rotation_node = node->InsertNewChildElement("Rotation");
    rotation_node->SetAttribute("X", m_rotation.x);
    rotation_node->SetAttribute("Y", m_rotation.y);
    rotation_node->SetAttribute("Z", m_rotation.z);
    rotation_node->SetAttribute("W", m_rotation.w);

    auto scale_node = node->InsertNewChildElement("Scale");
    scale_node->SetAttribute("X", m_scale.x);
    scale_node->SetAttribute("Y", m_scale.y);
    scale_node->SetAttribute("Z", m_scale.z);
}

void Transform::Deserialize(const tinyxml2::XMLElement *node)
{
    node->FirstChildElement("Position")->QueryFloatAttribute("X", &m_position.x);
    node->FirstChildElement("Position")->QueryFloatAttribute("Y", &m_position.y);
    node->FirstChildElement("Position")->QueryFloatAttribute("Z", &m_position.z);

    node->FirstChildElement("Rotation")->QueryFloatAttribute("X", &m_rotation.x);
    node->FirstChildElement("Rotation")->QueryFloatAttribute("Y", &m_rotation.y);
    node->FirstChildElement("Rotation")->QueryFloatAttribute("Z", &m_rotation.z);
    node->FirstChildElement("Rotation")->QueryFloatAttribute("W", &m_rotation.w);

    node->FirstChildElement("Scale")->QueryFloatAttribute("X", &m_scale.x);
    node->FirstChildElement("Scale")->QueryFloatAttribute("Y", &m_scale.y);
    node->FirstChildElement("Scale")->QueryFloatAttribute("Z", &m_scale.z);
}
} // namespace Aurora