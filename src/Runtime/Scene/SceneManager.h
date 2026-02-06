#pragma once
// std include
#include <memory>
#include <vector>
#include <unordered_map>
// thirdparty include

// Aurora include
#include "Runtime/Scene/SceneObjects/SceneObject.h"
#include "Runtime/Scene/Octree.h"
#include "Runtime/Scene/BoundingVolumeHierarchy.h"
#include "Runtime/Scene/Scene.h"
#include "Utility/Serializable.h"

namespace Aurora
{
class ViewFrustum;

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
    void SaveSceneAs(const std::string& scene_path);
    void CreateNewScene(const std::string& save_path);

    void Update();

    const std::shared_ptr<Scene> GetScene() const { return m_scene; }
    const std::vector<glm::vec3> GetOctreeVertices() const;
    const std::vector<glm::vec3> GetBVHVertices() const;

    MeshID RegisterMesh(const std::shared_ptr<Mesh>& mesh);
    void UnregisterMesh(MeshID id) { m_meshes.erase(id); }
    const std::unordered_map<MeshID, std::weak_ptr<Mesh>>& GetMeshes() const { return m_meshes; }
    std::vector<std::shared_ptr<Mesh>> GetMeshesInViewFrustum() const;

    // This is used for scene irrelavant objects, such as gizmos.
    std::shared_ptr<SceneObject> GetDummySceneObject() const { return m_dummy_scene_object; }

private:
    SceneManager() { m_dummy_scene_object = std::make_shared<SceneObject>("Dummy Scene Object"); }

private:
    std::shared_ptr<Scene> m_scene;
    std::unique_ptr<Octree> m_octree;
    std::unique_ptr<BoundingVolumeHierarchy> m_bvh;
    std::unordered_map<MeshID, std::weak_ptr<Mesh>> m_meshes;

    // This is used for scene irrelavant objects, such as gizmos.
    std::shared_ptr<SceneObject> m_dummy_scene_object;
};
} // namespace Aurora
