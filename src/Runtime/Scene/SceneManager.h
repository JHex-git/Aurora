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

    void LoadScene(const std::string& scene_path);
    void SaveScene();

    void Update();

    const std::shared_ptr<Scene> GetScene() const { return m_scene; }

    // This is used for scene irrelavant objects, such as gizmos.
    std::shared_ptr<SceneObject> GetDummySceneObject() const { return m_dummy_scene_object; }

private:
    SceneManager() { m_dummy_scene_object = std::make_shared<SceneObject>("Dummy Scene Object"); }

private:
    std::shared_ptr<Scene> m_scene;

    // This is used for scene irrelavant objects, such as gizmos.
    std::shared_ptr<SceneObject> m_dummy_scene_object;
};
} // namespace Aurora
