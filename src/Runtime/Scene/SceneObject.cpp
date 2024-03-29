// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/SceneObject.h"

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

    for (auto& component : m_components)
    {
        auto child_node = node->InsertNewChildElement(nullptr);
        component->Serialize(child_node);
    }
    for (auto& child : m_children)
    {
        auto child_node = node->InsertNewChildElement(nullptr);
        child->Serialize(child_node);
    }
}

void SceneObject::Deserialize(const tinyxml2::XMLElement *node)
{
    m_components.clear();

    m_name = node->Attribute("Name");
    auto p_child = node->FirstChildElement();
    while (p_child != nullptr)
    {
        if (!strcmp(p_child->Name(), "SceneObject"))
        {
            auto scene_object = std::make_shared<SceneObject>();
            scene_object->Deserialize(p_child);
            m_children.push_back(scene_object);
        }
        else // Component
        {
            std::shared_ptr<Component> component_ptr;
            if (!strcmp(p_child->Name(), "TransformComponent")) // ensure the transform component is the first one.
            {
                component_ptr = std::make_shared<Transform>();
                component_ptr->Deserialize(p_child);
                m_components.insert(m_components.begin(), component_ptr);
            }
            else
            {
                // TODO: Add Component here
                if (!strcmp(p_child->Name(), "MeshRendererComponent")) component_ptr = std::make_shared<MeshRenderer>();
                else
                {
                    spdlog::error("Unknown component {}.", p_child->Name());
                }

                if (component_ptr != nullptr) 
                {
                    component_ptr->Deserialize(p_child);
                    m_components.push_back(component_ptr);
                }
            }
        }

        p_child = p_child->NextSiblingElement();
    }
}
} // namespace Aurora