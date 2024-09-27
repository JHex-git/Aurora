// std include

// thirdparty include

// Aurora include
#include "Math/ViewFrustum.h"

namespace Aurora
{

ViewFrustum::ViewFrustum(float fov, float aspect_ratio, float near_plane, float far_plane)
{
    // Compute the near and far plane dimensions.
    float double_tan_half_fov = 2.f * tan(glm::radians(fov / 2.f));
    float near_height = double_tan_half_fov * near_plane;
    float near_width = near_height * aspect_ratio;
    float far_height = double_tan_half_fov * far_plane;
    float far_width = far_height * aspect_ratio;

    // Compute the center of the near and far planes.
    glm::vec3 center_near = position + lookat * near_plane;
    glm::vec3 center_far = position + lookat * far_plane;

    // Compute the normal of the near and far planes.
    glm::vec3 normal_near = -lookat;
    glm::vec3 normal_far = lookat;

    // Compute the 4 corners of the near and far planes.
    glm::vec3 right = -glm::normalize(glm::cross(up, lookat));
    m_corners[0] = center_near - right * near_width / 2.f - up * near_height / 2.f;
    m_corners[1] = center_near + right * near_width / 2.f - up * near_height / 2.f;
    m_corners[2] = center_near + right * near_width / 2.f + up * near_height / 2.f;
    m_corners[3] = center_near - right * near_width / 2.f + up * near_height / 2.f;
    m_corners[4] = center_far - right * far_width / 2.f - up * far_height / 2.f;
    m_corners[5] = center_far + right * far_width / 2.f - up * far_height / 2.f;
    m_corners[6] = center_far + right * far_width / 2.f + up * far_height / 2.f;
    m_corners[7] = center_far - right * far_width / 2.f + up * far_height / 2.f;

    // Compute the 6 planes of the frustum.
    m_planes[0] = glm::vec4(normal_near, -near_plane); // near
    m_planes[1] = glm::vec4(normal_far, far_plane); // far
    m_planes[2] = glm::vec4(glm::normalize(glm::cross(m_corners[4] - m_corners[0], m_corners[1] - m_corners[0])), 0); // bottom
    m_planes[3] = glm::vec4(glm::normalize(glm::cross(m_corners[5] - m_corners[1], m_corners[2] - m_corners[1])), 0); // right
    m_planes[4] = glm::vec4(glm::normalize(glm::cross(m_corners[6] - m_corners[2], m_corners[3] - m_corners[2])), 0); // top
    m_planes[5] = glm::vec4(glm::normalize(glm::cross(m_corners[7] - m_corners[3], m_corners[0] - m_corners[3])), 0); // left
}


bool Intersect(const ViewFrustum& frustum, const AxisAlignedBoundingBox& aabb)
{
    // refer to https://iquilezles.org/articles/frustumcorrect/
    // check box outside/inside of frustum
    const auto aabb_min = aabb.GetMin();
    const auto aabb_max = aabb.GetMax();
    for (const auto& plane : frustum.m_planes)
    {
        int outside_frustum_count = 0;
        outside_frustum_count += glm::dot(plane, glm::vec4(aabb_min, -1)) > 0 ? 1 : 0;
        outside_frustum_count += glm::dot(plane, glm::vec4(aabb_max.x, aabb_min.y, aabb_min.z, -1)) > 0 ? 1 : 0;
        outside_frustum_count += glm::dot(plane, glm::vec4(aabb_min.x, aabb_max.y, aabb_min.z, -1)) > 0 ? 1 : 0;
        outside_frustum_count += glm::dot(plane, glm::vec4(aabb_max.x, aabb_max.y, aabb_min.z, -1)) > 0 ? 1 : 0;
        outside_frustum_count += glm::dot(plane, glm::vec4(aabb_min.x, aabb_min.y, aabb_max.z, -1)) > 0 ? 1 : 0;
        outside_frustum_count += glm::dot(plane, glm::vec4(aabb_max.x, aabb_min.y, aabb_max.z, -1)) > 0 ? 1 : 0;
        outside_frustum_count += glm::dot(plane, glm::vec4(aabb_min.x, aabb_max.y, aabb_max.z, -1)) > 0 ? 1 : 0;
        outside_frustum_count += glm::dot(plane, glm::vec4(aabb_max, -1)) > 0 ? 1 : 0;
        if (outside_frustum_count == 8)
        {
            return false;
        }
    }

    // check frustum outside/inside box
    int out = 0;
    for (int i = 0; i < 8; i++) 
        out += ((frustum.m_corners[i].x > aabb_max.x) ? 1 : 0);
    if (out == 8) return false;
    out=0;
    for (int i = 0; i < 8; i++)
        out += ((frustum.m_corners[i].x < aabb_min.x) ? 1 : 0);
    if (out == 8) return false;
    out=0;
    for (int i = 0; i < 8; i++)
        out += ((frustum.m_corners[i].y > aabb_max.y) ? 1 : 0);
    if (out == 8) return false;
    out=0;
    for (int i = 0; i < 8; i++)
        out += ((frustum.m_corners[i].y < aabb_min.y) ? 1 : 0);
    if (out == 8) return false;
    out=0;
    for (int i = 0; i < 8; i++)
        out += ((frustum.m_corners[i].z > aabb_max.z) ? 1 : 0);
    if (out == 8) return false;
    out=0;
    for (int i = 0; i < 8; i++)
        out += ((frustum.m_corners[i].z < aabb_min.z) ? 1 : 0);
    if (out == 8) return false;

    return true;
}

bool Intersect(const AxisAlignedBoundingBox& aabb, const ViewFrustum& frustum)
{
    return Intersect(frustum, aabb);
}
} // namespace Aurora