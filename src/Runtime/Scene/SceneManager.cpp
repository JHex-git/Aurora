// std include

// thirdparty include
#include "thirdparty/tinyxml2/tinyxml2.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Runtime/Scene/SceneManager.h"
#include "Runtime/Scene/Mesh.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "Utility/FileSystem.h"

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

void SceneManager::LoadMesh(std::string file_path)
{
    if (m_scene)
    {
        m_scene->LoadMesh(file_path);
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

    m_scene = std::make_unique<Scene>(scene_path);
    tinyxml2::XMLDocument doc;
    doc.LoadFile(scene_path.c_str());
    auto root = doc.RootElement();
    m_scene->Deserialize(root);
}
} // namespace Aurora