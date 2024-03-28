#pragma once
// std include
#include <memory>
#include <vector>
// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Runtime/Scene/Component.h"
#include "Core/Render/Pass/RenderPass.h"
#include "Runtime/Scene/Mesh.h"
#include "Runtime/Scene/MeshRenderer.h"

namespace Aurora
{

class SceneObject : public Serializable
{
public:
    SceneObject() = default;
    ~SceneObject() = default;

    void AddComponent(std::shared_ptr<Component> component)
    {
        m_components.push_back(component);
    }

    void Update()
    {
        for (auto& component : m_components)
        {
            component->Update();
        }
    }

    void Serialize(tinyxml2::XMLElement *node) override
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

    void Deserialize(const tinyxml2::XMLElement *node) override
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

            p_child = p_child->NextSiblingElement();
        }
    }

    const std::string GetName() const { return m_name; }
    const std::vector<std::shared_ptr<Component>>& GetComponents() const { return m_components; }
    const std::vector<std::shared_ptr<SceneObject>>& GetChildren() const { return m_children; }

private:
    std::vector<std::shared_ptr<Component>> m_components;
    std::vector<std::shared_ptr<SceneObject>> m_children;
    std::string m_name;
};
} // namespace Aurora