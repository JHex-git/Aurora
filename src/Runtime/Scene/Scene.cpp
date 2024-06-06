// std include

// thirdparty include
#include "thirdparty/tinyxml2/tinyxml2.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Runtime/Scene/Scene.h"
#include "Runtime/Scene/Mesh.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/MeshRenderer.h"

namespace Aurora
{

Scene::Scene(std::string&& scene_path)
    : m_scene_path(std::move(scene_path))
{
    m_scene_name = FileSystem::GetFileName(m_scene_path);
}

Scene::Scene(const std::string& scene_path)
    : m_scene_path(scene_path)
{
    m_scene_name = FileSystem::GetFileName(m_scene_path);
}

void Scene::LoadMesh(std::string file_path)
{
    auto scene_object = std::make_shared<SceneObject>();

    auto mesh_renderer = std::make_shared<MeshRenderer>();
    mesh_renderer->Init(scene_object);
    mesh_renderer->LoadMesh(file_path);

    scene_object->AddComponent(mesh_renderer);
    m_scene_objects.push_back(std::move(scene_object));
}

void Scene::Update()
{
    for (auto& scene_object : m_scene_objects)
    {
        scene_object->Update();
    }
}

void Scene::Serialize(tinyxml2::XMLElement *node)
{
    for (auto& scene_object : m_scene_objects)
    {
        auto child_node = node->InsertNewChildElement("Temp");
        scene_object->Serialize(child_node);
    }
}

void Scene::Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner)
{
    m_scene_objects.clear();

    m_scene_name = node->Attribute("Name");
    auto p_child = node->FirstChildElement();
    while (p_child != nullptr)
    {
        if (!strcmp(p_child->Name(), "SceneObject"))
        {
            auto scene_object = std::make_shared<SceneObject>();
            scene_object->Deserialize(p_child, nullptr);
            m_scene_objects.push_back(scene_object);
        }
        else
        {
            spdlog::error("Unknown element {} in scene file.", p_child->Name());
        }
        p_child = p_child->NextSiblingElement();
    }
    spdlog::info("Scene {} loaded.", m_scene_name);
}

void Scene::Save()
{
    if (m_is_dirty)
    {
        tinyxml2::XMLDocument doc;
        auto root = doc.NewElement("Scene");
        doc.InsertEndChild(root);
        root->SetAttribute("Name", m_scene_name.c_str());
        Serialize(root);
        doc.SaveFile(m_scene_path.c_str());
        m_is_dirty = false;
        spdlog::info("Scene {} saved.", m_scene_name);
    }
}
} // namespace Aurora