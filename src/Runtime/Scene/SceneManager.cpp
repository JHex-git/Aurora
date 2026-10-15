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
namespace
{
    constexpr std::size_t kRebuildMovedMeshThreshold = 8;
} // namespace

SceneManager::~SceneManager()
{
}

void SceneManager::CleanUp()
{
    m_scene.reset();
    m_octree.reset();
    m_bvh.reset();
    m_meshes.clear();
    m_bvh_leaf_map.clear();
    m_moved_meshes.clear();
    m_bvh_dirty = true;
    m_dummy_scene_object.reset();
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
    m_bvh_dirty = true;

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
    IndexBVH();
    m_moved_meshes.clear();
    m_bvh_dirty = false;
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

const std::vector<glm::vec3> SceneManager::GetBVHVertices()
{
    UpdateBVH();
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
    m_bvh_dirty = true;
    return id++;
}

void SceneManager::UnregisterMesh(MeshID id)
{
    m_meshes.erase(id);
    m_bvh_dirty = true;
}

void SceneManager::MarkMeshMoved(MeshID id)
{
    if (id != Mesh::INVALID_MESH_ID)
    {
        m_moved_meshes.insert(id);
    }
}

void SceneManager::UpdateBVH()
{
    if (!m_bvh_dirty)
    {
        // Refitting keeps the topology the tree was built with: it stays correct, but the
        // splits stop being a good fit once most of the scene has moved, so rebuild then.
        const bool rebuild_preferred = m_moved_meshes.size() >= kRebuildMovedMeshThreshold &&
            m_moved_meshes.size() * 2 >= m_bvh_leaf_map.size();
        if (!rebuild_preferred)
        {
            RefitBVHForMovedMeshes();
            return;
        }
        m_bvh_dirty = true;
    }

    if (m_scene)
    {
        m_bvh = std::make_unique<BoundingVolumeHierarchy>(BoundingVolumeHierarchy::Build(m_scene->GetSceneObjects()));
        IndexBVH();
    }
    else
    {
        m_bvh.reset();
        m_bvh_leaf_map.clear();
    }

    m_moved_meshes.clear();
    m_bvh_dirty = false;
}

void SceneManager::IndexBVH()
{
    m_bvh_leaf_map.clear();
    if (!m_bvh)
        return;

    std::vector<BoundingVolumeHierarchy*> stack;
    m_bvh->m_parent = nullptr;
    stack.push_back(m_bvh.get());
    while (!stack.empty())
    {
        auto* node = stack.back();
        stack.pop_back();
        if (node->m_child1)
        {
            node->m_child1->m_parent = node;
            stack.push_back(node->m_child1.get());
        }
        if (node->m_child2)
        {
            node->m_child2->m_parent = node;
            stack.push_back(node->m_child2.get());
        }
        if (!node->m_child1 && !node->m_child2)
        {
            if (auto mesh = node->m_mesh.lock())
            {
                m_bvh_leaf_map[mesh->GetID()] = node;
            }
        }
    }
}

void SceneManager::RefitBVHForMovedMeshes()
{
    if (!m_bvh || m_moved_meshes.empty())
        return;

    for (const auto& mesh_id : m_moved_meshes)
    {
        auto it = m_bvh_leaf_map.find(mesh_id);
        if (it == m_bvh_leaf_map.end())
            continue;

        auto* leaf = it->second;
        auto mesh = leaf->m_mesh.lock();
        if (!mesh)
            continue;

        leaf->m_aabb = mesh->GetAABB();

        auto* node = leaf->m_parent;
        while (node)
        {
            if (node->m_child1 && node->m_child2)
            {
                node->m_aabb = node->m_child1->m_aabb | node->m_child2->m_aabb;
            }
            else if (auto node_mesh = node->m_mesh.lock())
            {
                node->m_aabb = node_mesh->GetAABB();
            }
            node = node->m_parent;
        }
    }

    m_moved_meshes.clear();
}

std::vector<std::shared_ptr<Mesh>> SceneManager::GetMeshesInViewFrustum()
{
    UpdateBVH();
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
                    auto mesh = node->m_mesh.lock();
                    if (mesh)
                    {
                        meshes.push_back(mesh);
                    }
                }
            }
        }
    }
    return meshes;
}

void SceneManager::CreateNewScene(const std::string& save_path)
{
    TextureManager::GetInstance().Reset();
    m_bvh_dirty = true;

    m_scene = std::make_shared<Scene>(save_path);
    m_scene->SetDirty();
    WindowSystem::GetInstance().UpdateTitleSurfix(" - " + save_path);
}
} // namespace Aurora
