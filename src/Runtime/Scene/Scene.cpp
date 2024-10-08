// std include

// thirdparty include
#include "thirdparty/tinyxml2/tinyxml2.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Runtime/Scene/Scene.h"
#include "Runtime/Scene/Components/Mesh.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/SceneObjects/SceneObjectFactory.h"
#include "Runtime/Scene/LightManager.h"
#include "Runtime/Scene/Camera.h"
#include "Core/Render/RenderSystem.h"
#include "Core/Render/WindowSystem.h"

namespace Aurora
{

Scene::Scene(std::string&& scene_path)
    : m_scene_path(std::move(scene_path)) { }

Scene::Scene(const std::string& scene_path)
    : m_scene_path(scene_path) { }

void Scene::CreateEmptySceneObject()
{
    m_scene_objects.push_back(std::make_shared<SceneObject>("Empty Scene Object"));
    SetSelectedSceneObject(m_scene_objects.back());
    SetDirty();
}

void Scene::LoadMesh(std::string file_path)
{
    m_scene_objects.push_back(SceneObjectFactory::CreateMesh(file_path));
    SetSelectedSceneObject(m_scene_objects.back());
    SetDirty();
}

void Scene::AddSkybox(std::array<std::string, 6>&& skybox_paths)
{
    m_scene_objects.push_back(SceneObjectFactory::CreateSkybox(std::move(skybox_paths)));
    SetSelectedSceneObject(m_scene_objects.back());
    SetDirty();
}

void Scene::AddPointLight()
{
    m_scene_objects.push_back(SceneObjectFactory::CreatePointLight());
    SetSelectedSceneObject(m_scene_objects.back());
    SetDirty();
}

void Scene::AddDirectionalLight()
{
    m_scene_objects.push_back(SceneObjectFactory::CreateDirectionalLight());
    SetSelectedSceneObject(m_scene_objects.back());
    SetDirty();
}

void Scene::SetSelectedSceneObject(std::shared_ptr<SceneObject> scene_object)
{
    if (m_selected_scene_object.lock() != scene_object)
    {
        m_selected_scene_object = scene_object;
        RenderSystem::GetInstance().OnSelectedSceneObjectChange();
    }
}

void Scene::Update()
{
    for (auto& scene_object : m_scene_objects)
    {
        scene_object->Update();
    }
    LightManager::GetInstance().UpdateActiveLights(MainCamera::GetInstance().GetPosition());
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
}

void Scene::SetDirty()
{
    if (m_is_dirty)
    {
        return;
    }
    m_is_dirty = true;
    WindowSystem::GetInstance().UpdateTitlePrefix(" *");
}

void Scene::Save()
{
    if (m_is_dirty)
    {
        tinyxml2::XMLDocument doc;
        auto root = doc.NewElement("Scene");
        doc.InsertEndChild(root);
        Serialize(root);
        doc.SaveFile(m_scene_path.c_str());
        m_is_dirty = false;
        spdlog::info("Scene {} saved.", m_scene_path);
        WindowSystem::GetInstance().UpdateTitlePrefix("");
    }
}

void Scene::DeleteSelectedSceneObject()
{
    if (auto selected_scene_object = m_selected_scene_object.lock())
    {
        m_scene_objects.erase(std::remove(m_scene_objects.begin(), m_scene_objects.end(), selected_scene_object), m_scene_objects.end());
        m_selected_scene_object.reset();
        SetDirty();
    }
}
} // namespace Aurora