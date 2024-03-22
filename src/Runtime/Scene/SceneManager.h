#pragma once
// std include
#include <memory>
#include <vector>
// thirdparty include

// Aurora include
#include "Runtime/Scene/SceneObject.h"
#include "Runtime/Scene/Scene.h"
#include "Utility/Serializable.h"

namespace Aurora
{

class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager();

    void LoadMesh(std::string file_path);

    void LoadScene(const std::string& scene_path);
    void SaveScene();

    void Update();

private:
    std::unique_ptr<Scene> m_scene;
};
} // namespace Aurora
