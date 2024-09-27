// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/Octree.h"
#include "Runtime/Scene/SceneObjects/SceneObject.h"
#include "Runtime/Scene/Components/MeshRenderer.h"

namespace Aurora
{

Octree Octree::Build(const std::vector<std::shared_ptr<SceneObject>>& scene_objects)
{
    // Octree tree(AxisAlignedBoundingBox(glm::vec3(-1000), glm::vec3(1000)));
    Octree tree(AxisAlignedBoundingBox(glm::vec3(-50), glm::vec3(50)));
    for (const auto& scene_object : scene_objects)
    {
        tree.Insert(scene_object);
    }
    return tree;
}

void Octree::Insert(std::shared_ptr<SceneObject> scene_object)
{
    const static std::vector<glm::vec3> offset{
        glm::vec3(0, 0, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(1, 1, 0),
        glm::vec3(0, 0, 1),
        glm::vec3(1, 0, 1),
        glm::vec3(0, 1, 1),
        glm::vec3(1, 1, 1),
    };

    if (scene_object)
    {
        const auto mesh_renderer = scene_object->TryGetComponent<MeshRenderer>();
        if (mesh_renderer)
        {
            // Test if children can hold the AABB
            const auto mesh = mesh_renderer->m_mesh;
            const auto mesh_aabb = mesh->GetAABB();
            const auto mesh_extent = mesh_aabb.GetMax() - mesh_aabb.GetMin();
            float mesh_max_extent = std::max(mesh_extent.x, std::max(mesh_extent.y, mesh_extent.z));
            float extent = m_aabb.GetMax().x - m_aabb.GetMin().x;

            if (mesh_max_extent > extent / 2.0) // There is no possibility that the mesh can be hold by the children
            {
                // if not, hold the mesh
                m_meshes.push_back(mesh);
            }
            else
            {
                int container_index = -1;
                float half_extent = extent / 2.0;
                for (int i = 0; i < 8; i++)
                {
                    const auto child_aabb_min = m_aabb.GetMin() + offset[i] * glm::vec3{half_extent};
                    AxisAlignedBoundingBox child_aabb(child_aabb_min, child_aabb_min + glm::vec3{half_extent});
                    if (mesh_aabb.IsInside(child_aabb))
                    {
                        container_index = i;
                        break;
                    }
                }

                if (container_index == -1) // No suitable children can hold the mesh
                {
                    m_meshes.push_back(mesh);
                }
                else
                {
                    // insert the mesh to the children recursively
                    if (!m_children)
                    {
                        m_children = std::array<std::unique_ptr<Octree>, 8>();
                        for (int j = 0; j < 8; j++)
                        {
                            const auto child_aabb_min = m_aabb.GetMin() + offset[j] * glm::vec3{half_extent};
                            AxisAlignedBoundingBox child_aabb(child_aabb_min, child_aabb_min + glm::vec3{half_extent});
                            m_children->at(j) = std::make_unique<Octree>(Octree(std::move(child_aabb)));
                        }
                    }
                    m_children->at(container_index)->Insert(scene_object);
                }
            }
        }
    }
}
} // namespace Aurora