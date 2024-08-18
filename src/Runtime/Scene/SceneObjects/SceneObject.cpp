// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/SceneObjects/SceneObject.h"
#include "Runtime/Scene/Components/MeshRenderer.h"
#include "Runtime/Scene/Components/SkyboxRenderer.h"
#include "Runtime/Scene/Components/Light.h"
#include "Runtime/Scene/SceneManager.h"

namespace Aurora
{

void SceneObject::AddComponent(std::shared_ptr<Component> component)
{
    m_components.push_back(component);
}

void SceneObject::Update()
{
    for (auto& component : m_components)
    {
        component->Update();
    }
}

void SceneObject::Serialize(tinyxml2::XMLElement *node)
{
    node->SetName("SceneObject");
    node->SetAttribute("Name", m_name.c_str());

    {
        auto child_node = node->InsertNewChildElement("Temp");
        m_transform->Serialize(child_node);
    }
    for (auto& component : m_components)
    {
        auto child_node = node->InsertNewChildElement("Temp");
        component->Serialize(child_node);
    }
    for (auto& child : m_children)
    {
        auto child_node = node->InsertNewChildElement("Temp");
        child->Serialize(child_node);
    }
}

void SceneObject::Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner)
{
    m_parent = owner;
    m_components.clear();

    m_name = node->Attribute("Name");
    auto p_child = node->FirstChildElement();
    while (p_child != nullptr)
    {
        if (!strcmp(p_child->Name(), "SceneObject"))
        {
            auto scene_object = std::make_shared<SceneObject>();
            scene_object->Deserialize(p_child, shared_from_this());
            m_children.push_back(scene_object);
        }
        else // Component
        {
            if (!strcmp(p_child->Name(), "Transform"))
            {
                m_transform->Deserialize(p_child, shared_from_this());
            }
            else
            {
                std::shared_ptr<Component> component_ptr;
                // TODO: Add Component here
                if (!strcmp(p_child->Name(), "MeshRendererComponent")) component_ptr = std::make_shared<MeshRenderer>();
                else if (!strcmp(p_child->Name(), "SkyboxRendererComponent")) component_ptr = std::make_shared<SkyboxRenderer>();
                else if (!strcmp(p_child->Name(), "LightComponent")) component_ptr = std::make_shared<Light>();
                else
                {
                    spdlog::error("Unknown component {}.", p_child->Name());
                }

                if (component_ptr != nullptr) 
                {
                    component_ptr->Deserialize(p_child, shared_from_this());
                    m_components.push_back(component_ptr);
                }
            }
        }

        p_child = p_child->NextSiblingElement();
    }
}

void SceneObject::SetName(const std::string& name)
{
    m_name = name;
    auto scene = SceneManager::GetInstance().GetScene();
    if (scene != nullptr) scene->SetDirty();
}
} // namespace Aurora