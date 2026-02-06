// std include
#include <list>
// thirdparty include
#include "thirdparty/tinyxml2/tinyxml2.h"
// Aurora include
#include "Runtime/Scene/SceneManager.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/TextureManager.h"
#include "Core/Render/WindowSystem.h"
#include "Runtime/Scene/Camera.h"

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

void SceneManager::SaveSceneAs(const std::string& scene_path)
{
    if (m_scene)
    {
        m_scene->SetScenePath(scene_path);
        m_scene->Save();
        WindowSystem::GetInstance().UpdateTitleSurfix(" - " + scene_path);
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
    TextureManager::GetInstance().Reset();
    m_meshes.clear();

    spdlog::info("Scene {} loading...", scene_path);
    m_scene = std::make_shared<Scene>(scene_path);
    tinyxml2::XMLDocument doc;
    doc.LoadFile(scene_path.c_str());
    auto root = doc.RootElement();
    m_scene->Deserialize(root, nullptr);
    spdlog::info("Scene {} loaded.", scene_path);
    
    // m_octree = std::make_unique<Octree>(Octree::Build(m_scene->GetSceneObjects()));
    // spdlog::info("Spatial Octree built.");
    m_bvh = std::make_unique<BoundingVolumeHierarchy>(BoundingVolumeHierarchy::Build(m_scene->GetSceneObjects()));
    spdlog::info("Bounding Volume Hierarchy built.");

    WindowSystem::GetInstance().UpdateTitleSurfix(" - " + scene_path);
}

const std::vector<glm::vec3> SceneManager::GetOctreeVertices() const
{
    std::vector<glm::vec3> octree_aabbs;
    std::list<Octree*> nodes;
    nodes.push_back(m_octree.get());
    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop_front();
        octree_aabbs.push_back(node->m_aabb.GetMin());
        octree_aabbs.push_back(node->m_aabb.GetMax());
        if (node->m_children)
        {
            for (const auto& child : *node->m_children)
            {
                nodes.push_back(child.get());
            }
        }
    }
    return octree_aabbs;
}

const std::vector<glm::vec3> SceneManager::GetBVHVertices() const
{
    std::vector<glm::vec3> bvh_aabbs;
    std::list<BoundingVolumeHierarchy*> nodes;
    nodes.push_back(m_bvh.get());
    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop_front();
        bvh_aabbs.push_back(node->m_aabb.GetMin());
        bvh_aabbs.push_back(node->m_aabb.GetMax());
        if (node->m_child1)
        {
            nodes.push_back(node->m_child1.get());
        }
        if (node->m_child2)
        {
            nodes.push_back(node->m_child2.get());
        }
    }
    return bvh_aabbs;
}

MeshID SceneManager::RegisterMesh(const std::shared_ptr<Mesh>& mesh)
{
    static MeshID id = 1;
    m_meshes.emplace(id, mesh);
    return id++;
}

std::vector<std::shared_ptr<Mesh>> SceneManager::GetMeshesInViewFrustum() const
{
    std::vector<std::shared_ptr<Mesh>> meshes;
    if (m_scene)
    {
        std::list<BoundingVolumeHierarchy*> bvh_nodes;
        if (BoundingVolumeHierarchy *  bvh_node = m_bvh.get()) bvh_nodes.push_back(bvh_node);
        BoundingVolumeHierarchy* node = nullptr;
        while (!bvh_nodes.empty())
        {
            node = bvh_nodes.front();
            bvh_nodes.pop_front();
            if (MainCamera::GetInstance().Intersect(node->m_aabb))
            {
                if (node->m_child1 && node->m_child2)
                {
                    bvh_nodes.push_back(node->m_child1.get());
                    bvh_nodes.push_back(node->m_child2.get());
                }
                else
                {
                    meshes.push_back(node->m_mesh.lock());
                }
            }
        }
    }
    return meshes;
}

void SceneManager::CreateNewScene(const std::string& save_path)
{
    TextureManager::GetInstance().Reset();

    m_scene = std::make_shared<Scene>(save_path);
    m_scene->SetDirty();
    WindowSystem::GetInstance().UpdateTitleSurfix(" - " + save_path);
}
} // namespace Aurora