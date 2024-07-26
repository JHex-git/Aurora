#pragma once
// std include
#include <memory>
#include <vector>
// thirdparty include

// Aurora include
#include "Runtime/Scene/SceneObjects/SceneObject.h"
#include "Runtime/Scene/Scene.h"
#include "Utility/Serializable.h"

namespace Aurora
{

class SceneManager
{
public:
    static SceneManager& GetInstance()
    {
        static SceneManager instance;
        return instance;
    }

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

    ~SceneManager();

    void LoadMesh(std::string file_path);

    void LoadScene(const std::string& scene_path);
    void SaveScene();

    void Update();

    const std::shared_ptr<Scene> GetScene() const { return m_scene; }

private:
    SceneManager() = default;

private:
    std::shared_ptr<Scene> m_scene;
};
} // namespace Aurora
