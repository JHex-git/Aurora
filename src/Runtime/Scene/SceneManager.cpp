// std include

// thirdparty include
#include "thirdparty/tinyxml2/tinyxml2.h"
// Aurora include
#include "Runtime/Scene/SceneManager.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/TextureManager.h"

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

void SceneManager::Update()
{
    if (m_scene)
    {
        m_scene->Update();
    }
}

void SceneManager::LoadScene(const std::string& scene_path)
{
    // save current scene before load another scene
    SaveScene();
    TextureManager::GetInstance().Reset();

    m_scene = std::make_shared<Scene>(scene_path);
    tinyxml2::XMLDocument doc;
    doc.LoadFile(scene_path.c_str());
    auto root = doc.RootElement();
    m_scene->Deserialize(root, nullptr);
}
} // namespace Aurora