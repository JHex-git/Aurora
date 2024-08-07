#pragma once
// std include
#include <memory>
#include <vector>
// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Runtime/Scene/Components/Component.h"
#include "Core/Render/Pass/RenderPass.h"
#include "Runtime/Scene/Components/Mesh.h"
#include "Runtime/Scene/Components/Transform.h"

namespace Aurora
{

class SceneObject : public Serializable, public std::enable_shared_from_this<SceneObject>
{
public:
    SceneObject(std::string name = "", std::shared_ptr<SceneObject> parent = nullptr)
    {
        m_transform = std::make_shared<Transform>();
        m_parent = parent;
        m_name = name;
    }
    ~SceneObject() = default;

    void AddComponent(std::shared_ptr<Component> component);

    void Update();

    void Serialize(tinyxml2::XMLElement *node) override;

    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override;

    const std::string GetName() const { return m_name; }
    const std::vector<std::shared_ptr<Component>>& GetComponents() const { return m_components; }
    const std::vector<std::shared_ptr<SceneObject>>& GetChildren() const { return m_children; }
    std::shared_ptr<Transform> GetTransform() const { return m_transform; }

    template <typename T>
    std::shared_ptr<T> TryGetComponent()
    {
        for (auto component : m_components)
        {
            auto casted_component = std::dynamic_pointer_cast<T>(component);
            if (casted_component != nullptr) return casted_component;
        }
        return nullptr;
    }

private:
    std::shared_ptr<Transform> m_transform;
    std::vector<std::shared_ptr<Component>> m_components;
    std::vector<std::shared_ptr<SceneObject>> m_children;
    std::weak_ptr<SceneObject> m_parent;
    std::string m_name;
};
} // namespace Aurora