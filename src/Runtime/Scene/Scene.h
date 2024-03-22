#pragma once
// std include
#include <vector>
#include <memory>
// thirdparty include

// Aurora include
#include "Utility/Serializable.h"
#include "Runtime/Scene/SceneObject.h"

namespace Aurora
{

class Scene : public Serializable
{
public:
    Scene(std::string&& scene_path);
    Scene(const std::string& scene_path);
    ~Scene() = default;

    void Serialize(tinyxml2::XMLElement *node) override;
    void Deserialize(const tinyxml2::XMLElement *node) override;

    void Save();

    void Update();
    void LoadMesh(std::string file_path);

private:
    std::vector<std::shared_ptr<SceneObject>> m_scene_objects;
    std::string m_scene_path;
    std::string m_scene_name;
    bool m_is_dirty = false;
};
} // namespace Aurora