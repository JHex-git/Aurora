#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Runtime/Scene/SceneObjects/SceneObject.h"

namespace Aurora
{

class SceneObjectFactory
{
public:
    static std::shared_ptr<SceneObject> CreateMesh(const std::string& file_path);
    static std::shared_ptr<SceneObject> CreateSkybox(std::array<std::string, 6>&& skybox_paths);
    static std::shared_ptr<SceneObject> CreateLight();
};
} // namespace Aurora