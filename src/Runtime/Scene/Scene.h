#pragma once
// std include
#include <vector>
#include <memory>
// thirdparty include

// Aurora include
#include "Utility/Serializable.h"
#include "Runtime/Scene/SceneObjects/SceneObject.h"

namespace Aurora
{

class Scene : public Serializable
{
public:
    Scene(std::string&& scene_path);
    Scene(const std::string& scene_path);
    ~Scene() = default;

    void Serialize(tinyxml2::XMLElement *node) override;
    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override;

    void Save();

    void Update();
    void LoadMesh(std::string file_path);
    void CreateEmptySceneObject();
    void AddSkybox(std::array<std::string, 6>&& skybox_paths);
    void AddLight();

    void SetSelectedSceneObject(std::shared_ptr<SceneObject> scene_object) { m_selected_scene_object = scene_object; }
    void DeleteSelectedSceneObject();
    void SetDirty() { m_is_dirty = true; }

    const std::vector<std::shared_ptr<SceneObject>>& GetSceneObjects() const { return m_scene_objects; }
    const std::shared_ptr<SceneObject> GetSelectedSceneObject() const { return m_selected_scene_object.lock(); }

private:
    std::vector<std::shared_ptr<SceneObject>> m_scene_objects;
    std::weak_ptr<SceneObject> m_selected_scene_object;
    std::string m_scene_path;
    std::string m_scene_name;
    bool m_is_dirty = false;
};
} // namespace Aurora