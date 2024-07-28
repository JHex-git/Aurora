// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/SceneObjects/SceneObjectFactory.h"
#include "Runtime/Scene/Components/MeshRenderer.h"
#include "Runtime/Scene/Components/SkyboxRenderer.h"
#include "Runtime/Scene/Components/Light.h"
#include "Utility/FileSystem.h"

namespace Aurora
{

std::shared_ptr<SceneObject> SceneObjectFactory::CreateMesh(const std::string& file_path)
{
    auto scene_object = std::make_shared<SceneObject>(FileSystem::GetFileNameWithoutExtension(file_path));

    auto mesh_renderer = std::make_shared<MeshRenderer>();
    if (!mesh_renderer->Init(scene_object)) return nullptr;
    if (!mesh_renderer->LoadMesh(file_path)) return nullptr;

    scene_object->AddComponent(mesh_renderer);
    return std::move(scene_object);
}

std::shared_ptr<SceneObject> SceneObjectFactory::CreateSkybox(std::array<std::string, 6>&& skybox_paths)
{
    auto scene_object = std::make_shared<SceneObject>("Skybox");
    
    auto skybox_renderer = std::make_shared<SkyboxRenderer>();
    skybox_renderer->Init(scene_object);
    skybox_renderer->Load(std::move(skybox_paths));

    scene_object->AddComponent(skybox_renderer);
    return std::move(scene_object);
}

std::shared_ptr<SceneObject> SceneObjectFactory::CreateLight()
{
    auto scene_object = std::make_shared<SceneObject>("Light");

    auto light = std::make_shared<Light>(glm::vec3(1.0f));
    if (!light->Init(scene_object)) return nullptr;

    scene_object->AddComponent(light);
    return std::move(scene_object);
}

} // namespace Aurora