// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/BoundingVolumeHierarchy.h"
#include "Runtime/Scene/Components/MeshRenderer.h"

namespace Aurora
{

BoundingVolumeHierarchy BoundingVolumeHierarchy::Build(const std::vector<std::shared_ptr<SceneObject>>& scene_objects)
{
    BoundingVolumeHierarchy hierarchy;
    // Traverse the scene objects and extract all scene objects with mesh components
    // Get AABB union of all mesh objects simultaneously
    std::vector<std::shared_ptr<Mesh>> meshes;
    for (const auto& scene_object : scene_objects)
    {
        auto mesh_renderer = scene_object->TryGetComponent<MeshRenderer>();
        if (mesh_renderer)
        {
            const auto mesh = mesh_renderer->m_mesh;
            meshes.push_back(mesh);
            const auto&& mesh_aabb = mesh->GetAABB();
            hierarchy.m_aabb |= mesh_aabb;
        }
    }
    hierarchy.m_primitive_count = meshes.size();
    if (meshes.empty())
        return hierarchy;
    if (meshes.size() == 1)
    {
        hierarchy.m_mesh = meshes[0];
        hierarchy.m_aabb = meshes[0]->GetAABB();
        return hierarchy;
    }

    // Find the split axis
    int split_axis = 0;// x-axis
    auto hierarchy_extent = hierarchy.m_aabb.GetExtent();
    if (hierarchy_extent.y > hierarchy_extent[split_axis])
        split_axis = 1;
    if (hierarchy_extent.z > hierarchy_extent[split_axis])
        split_axis = 2;
    float split_pos = hierarchy.m_aabb.GetCenter()[split_axis];

    // Split the scene objects based on the split axis and their aabb centers
    int i = 0, j = meshes.size() - 1;
    while (i <= j)
    {
        const auto&& mesh_aabb = meshes[i]->GetAABB();
        auto center = mesh_aabb.GetCenter();
        if (center[split_axis] < split_pos)
            i++;
        else
            std::swap(meshes[i], meshes[j--]);
    }
    hierarchy.m_child1 = std::make_unique<BoundingVolumeHierarchy>(BuildChild(meshes, 0, i));
    hierarchy.m_child2 = std::make_unique<BoundingVolumeHierarchy>(BuildChild(meshes, i, meshes.size()));
    return hierarchy;
}

BoundingVolumeHierarchy BoundingVolumeHierarchy::BuildChild(std::vector<std::shared_ptr<Mesh>>& meshes, int begin, int end)
{
    BoundingVolumeHierarchy hierarchy;
    hierarchy.m_primitive_count = end - begin;
    if (hierarchy.m_primitive_count == 1)
    {
        hierarchy.m_mesh = meshes[begin];
        hierarchy.m_aabb = meshes[begin]->GetAABB();
        return hierarchy;
    }
    for (int i = begin; i < end; i++)
    {
        const auto&& mesh_aabb = meshes[i]->GetAABB();
        hierarchy.m_aabb |= mesh_aabb;
    }

    // Find the split axis
    int split_axis = 0;// x-axis
    auto hierarchy_extent = hierarchy.m_aabb.GetExtent();
    if (hierarchy_extent.y > hierarchy_extent[split_axis])
        split_axis = 1;
    if (hierarchy_extent.z > hierarchy_extent[split_axis])
        split_axis = 2;
    float split_pos = hierarchy.m_aabb.GetCenter()[split_axis];

    // Split the scene objects based on the split axis and their aabb centers
    int i = begin, j = end - 1;
    while (i <= j)
    {
        const auto&& mesh_aabb = meshes[i]->GetAABB();
        auto center = mesh_aabb.GetCenter();
        if (center[split_axis] < split_pos)
            i++;
        else
            std::swap(meshes[i], meshes[j--]);
    }
    hierarchy.m_child1 = std::make_unique<BoundingVolumeHierarchy>(BuildChild(meshes, begin, i));
    hierarchy.m_child2 = std::make_unique<BoundingVolumeHierarchy>(BuildChild(meshes, i, end));
    return hierarchy;
}
} // namespace Aurora