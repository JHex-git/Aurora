#pragma once
// std include
#include <array>
#include <memory>
// thirdparty include

// Aurora include
#include "Runtime/Scene/Components/Mesh.h"

namespace Aurora
{

class Octree
{
    friend class SceneManager;
public:
    static Octree Build(const std::vector<std::shared_ptr<SceneObject>>& scene_objects);

    Octree(Octree&& other)
    {
        m_children = std::move(other.m_children);
        m_aabb = std::move(other.m_aabb);
        m_meshes = std::move(other.m_meshes);
    }

    void Insert(std::shared_ptr<SceneObject> scene_object);
private:
    Octree(AxisAlignedBoundingBox aabb) : m_aabb(aabb) {}

    //           __________      __________     
    //          / |       /|    / |       /|    
    //         /__|_____ / |   /__|_____ / |    
    //         |  |_____|__|   |  |_____|__|    
    //         | /  6   | /    | /  7   | /     --
    //         |/_______|/     |/_______|/       /
    //    __________      __________            /
    //   / |       /|    / |       /|          / 
    //  /__|_____ / |   /__|_____ / |         /  -------> Top
    //  |  |_____|__|   |  |_____|__|        /   
    //  | /  4   | /    | /  5   | /        /    
    //  |/_______|/     |/_______|/       --     
    //           __________      __________     
    //          / |       /|    / |       /|    
    //         /__|_____ / |   /__|_____ / |    
    //         |  |_____|__|   |  |_____|__|    
    //         | /  2   | /    | /  3   | /     --
    //         |/_______|/     |/_______|/       /
    //    __________      __________            /
    //   / |       /|    / |       /|          / 
    //  /__|_____ / |   /__|_____ / |         / -------> Bottom
    //  |  |_____|__|   |  |_____|__|        /  
    //  | /  0   | /    | /  1   | /        /    
    //  |/_______|/     |/_______|/       --    
    std::optional<std::array<std::unique_ptr<Octree>, 8>> m_children;
    AxisAlignedBoundingBox m_aabb;
    std::vector<std::weak_ptr<Mesh>> m_meshes;
};
} // namespace Aurora