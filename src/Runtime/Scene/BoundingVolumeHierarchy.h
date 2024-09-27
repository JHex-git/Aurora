#pragma once
// std include
#include <memory>
#include <vector>
// thirdparty include

// Aurora include
#include "Runtime/Scene/SceneObjects/SceneObject.h"

namespace Aurora
{

class BoundingVolumeHierarchy
{
    friend class SceneManager;
public:
    static BoundingVolumeHierarchy Build(const std::vector<std::shared_ptr<SceneObject>>& scene_objects);

    BoundingVolumeHierarchy(BoundingVolumeHierarchy&& other)
    {
        m_child1 = std::move(other.m_child1);
        m_child2 = std::move(other.m_child2);
        m_primitive_count = other.m_primitive_count;
        other.m_primitive_count = 0;
        m_aabb = std::move(other.m_aabb);
        m_mesh = std::move(other.m_mesh);
    }
    void Update();
private:
    BoundingVolumeHierarchy() {}

    static BoundingVolumeHierarchy BuildChild(std::vector<std::shared_ptr<Mesh>>& meshes, int begin, int end);

    std::unique_ptr<BoundingVolumeHierarchy> m_child1 = nullptr;
    std::unique_ptr<BoundingVolumeHierarchy> m_child2 = nullptr;
    int m_primitive_count = 0;
    std::weak_ptr<Mesh> m_mesh;
    AxisAlignedBoundingBox m_aabb;
};
} // namespace Aurora