#pragma once
// std include
#include <array>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
// Aurora include
#include "Math/AxisAlignedBoundingBox.h"

namespace Aurora
{

/*** 
 * @description: ViewFrustum is in view space, with the camera at the origin and looking towards the negative z-axis.
 */
class ViewFrustum
{
public:
    /*** 
     * @param fov: Vertical field of view in degrees.
     */    
    ViewFrustum(float fov, float aspect_ratio, float near_plane, float far_plane);
    ~ViewFrustum() = default;

    friend bool Intersect(const ViewFrustum& frustum, const AxisAlignedBoundingBox& aabb);
    friend bool Intersect(const AxisAlignedBoundingBox& aabb, const ViewFrustum& frustum);

private:
    static constexpr glm::vec3 position = glm::vec3(0.0f);
    static constexpr glm::vec3 lookat = glm::vec3(0.0f, 0.0f, -1.0f);
    static constexpr glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // 6 planes of the frustum, in the order of near, far, bottom, right, top, left
    // xyz represent normal, towards the outside of the frustum
    // w represent the signed distance from the origin to the plane along the normal
    std::array<glm::vec4, 6> m_planes; // ax + by + cz - d = 0, ax1 + by1 + cz1 - d < 0 means the point is inside the frustum
    std::array<glm::vec3, 8> m_corners; // 8 corners of the frustum, first near, then far. For each plane, the first 4 corners are in the order of left-bottom, right-bottom, right-top, left-top.
};

bool Intersect(const ViewFrustum& frustum, const AxisAlignedBoundingBox& aabb);
bool Intersect(const AxisAlignedBoundingBox& aabb, const ViewFrustum& frustum);
} // namespace Aurora