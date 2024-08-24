// std include

// thirdparty include
#include "thirdparty/tinyxml2/tinyxml2.h"
// Aurora include
#include "Runtime/Scene/SceneManager.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/TextureManager.h"
#include "Core/Render/WindowSystem.h"

namespace Aurora
{
SceneManager::~SceneManager()
{
    SaveScene();
}

void SceneManager::SaveScene()
{
    if (m_scene)
    {
        m_scene->Save();
    }
}

void SceneManager::SaveSceneAs(const std::string& scene_path)
{
    if (m_scene)
    {
        m_scene->SetScenePath(scene_path);
        m_scene->Save();
        WindowSystem::GetInstance().UpdateTitleSurfix(" - " + scene_path);
    }
}

void SceneManager::Update()
{
    if (m_scene)
    {
        m_scene->Update();
    }
}

void SceneManager::LoadScene(const std::string& scene_path)
{
    TextureManager::GetInstance().Reset();

    spdlog::info("Scene {} loading...", scene_path);
    m_scene = std::make_shared<Scene>(scene_path);
    tinyxml2::XMLDocument doc;
    doc.LoadFile(scene_path.c_str());
    auto root = doc.RootElement();
    m_scene->Deserialize(root, nullptr);
    spdlog::info("Scene {} loaded.", scene_path);
    WindowSystem::GetInstance().UpdateTitleSurfix(" - " + scene_path);
}

void SceneManager::CreateNewScene(const std::string& save_path)
{
    TextureManager::GetInstance().Reset();

    m_scene = std::make_shared<Scene>(save_path);
    m_scene->SetDirty();
    WindowSystem::GetInstance().UpdateTitleSurfix(" - " + save_path);
}
} // namespace Aurora