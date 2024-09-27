#pragma once
// std include
#include <vector>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
// Aurora include


namespace Aurora
{

class AxisAlignedBoundingBox
{
public:
    AxisAlignedBoundingBox() = default;
    AxisAlignedBoundingBox(const std::vector<glm::vec3>& vertices);
    AxisAlignedBoundingBox(const glm::vec3& min, const glm::vec3& max);

    AxisAlignedBoundingBox(const AxisAlignedBoundingBox& aabb) = default;
    AxisAlignedBoundingBox& operator=(const AxisAlignedBoundingBox& aabb) = default;

    AxisAlignedBoundingBox(AxisAlignedBoundingBox&& aabb) = default;
    AxisAlignedBoundingBox& operator=(AxisAlignedBoundingBox&& aabb) = default;

    // Union
    AxisAlignedBoundingBox& operator|=(const AxisAlignedBoundingBox& aabb);
    // Union
    const AxisAlignedBoundingBox operator|(const AxisAlignedBoundingBox& aabb) const;
    const AxisAlignedBoundingBox operator|(AxisAlignedBoundingBox&& aabb) const;

    // Intersection
    AxisAlignedBoundingBox& operator&=(const AxisAlignedBoundingBox& aabb);
    // Intersection
    const AxisAlignedBoundingBox operator&(const AxisAlignedBoundingBox& aabb) const;
    const AxisAlignedBoundingBox operator&(AxisAlignedBoundingBox&& aabb) const;

    template<typename T> requires std::is_same_v<glm::mat4, std::decay_t<T>>
    friend const AxisAlignedBoundingBox operator*(T&& transform, const AxisAlignedBoundingBox& aabb);
    template<typename T> requires std::is_same_v<glm::mat4, std::decay_t<T>>
    friend const AxisAlignedBoundingBox operator*(const AxisAlignedBoundingBox& aabb, T&& transform);
    template<typename T> requires std::is_same_v<glm::mat4, std::decay_t<T>>
    friend const AxisAlignedBoundingBox operator*(T&& transform, AxisAlignedBoundingBox&& aabb);
    template<typename T> requires std::is_same_v<glm::mat4, std::decay_t<T>>
    friend const AxisAlignedBoundingBox operator*(AxisAlignedBoundingBox&& aabb, T&& transform);

    // friend const AxisAlignedBoundingBox operator*(const glm::mat4& transform, const AxisAlignedBoundingBox& aabb);
    // friend const AxisAlignedBoundingBox operator*(const AxisAlignedBoundingBox& aabb, const glm::mat4& transform);
    // friend const AxisAlignedBoundingBox operator*(const glm::mat4& transform, AxisAlignedBoundingBox&& aabb);
    // friend const AxisAlignedBoundingBox operator*(AxisAlignedBoundingBox&& aabb, const glm::mat4& transform);

    // friend const AxisAlignedBoundingBox operator*(glm::mat4&& transform, const AxisAlignedBoundingBox& aabb);
    // friend const AxisAlignedBoundingBox operator*(const AxisAlignedBoundingBox& aabb, glm::mat4&& transform);
    // friend const AxisAlignedBoundingBox operator*(glm::mat4&& transform, AxisAlignedBoundingBox&& aabb);
    // friend const AxisAlignedBoundingBox operator*(AxisAlignedBoundingBox&& aabb, glm::mat4&& transform);

    const bool IsInside(const AxisAlignedBoundingBox& aabb) const;

    const glm::vec3 GetMin() const { return m_min; }
    const glm::vec3 GetMax() const { return m_max; }
    const glm::vec3 GetCenter() const { return m_center; }
    const glm::vec3 GetExtent() const { return m_max - m_min; }
    const float GetExtentByAxis(int axis) const { return m_max[axis] - m_min[axis]; }
    const bool IsValid() const { return m_min.x <= m_max.x && m_min.y <= m_max.y && m_min.z <= m_max.z; }

private:
    glm::vec3 m_min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 m_max = glm::vec3(std::numeric_limits<float>::lowest());
    glm::vec3 m_center = glm::vec3(0.0f);
};

template<typename T> requires std::is_same_v<glm::mat4, std::decay_t<T>>
const AxisAlignedBoundingBox operator*(T&& transform, const AxisAlignedBoundingBox& aabb)
{
    std::vector<glm::vec3> vertices(8);
    vertices[0] = std::forward<T>(transform) * glm::vec4{aabb.m_min.x, aabb.m_min.y, aabb.m_max.z, 1};
    vertices[1] = std::forward<T>(transform) * glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_min.z, 1};
    vertices[2] = std::forward<T>(transform) * glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_min.z, 1};
    vertices[3] = std::forward<T>(transform) * glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_max.z, 1};
    vertices[4] = std::forward<T>(transform) * glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_max.z, 1};
    vertices[5] = std::forward<T>(transform) * glm::vec4{aabb.m_max.x, aabb.m_max.y, aabb.m_min.z, 1};
    vertices[6] = std::forward<T>(transform) * glm::vec4{aabb.m_min, 1};
    vertices[7] = std::forward<T>(transform) * glm::vec4{aabb.m_max, 1};

    return AxisAlignedBoundingBox(vertices);
}

template<typename T> requires std::is_same_v<glm::mat4, std::decay_t<T>>
const AxisAlignedBoundingBox operator*(const AxisAlignedBoundingBox& aabb, T&& transform)
{
    std::vector<glm::vec3> vertices(8);
    vertices[0] = glm::vec4{aabb.m_min.x, aabb.m_min.y, aabb.m_max.z, 1} * std::forward<T>(transform);
    vertices[1] = glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_min.z, 1} * std::forward<T>(transform);
    vertices[2] = glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_min.z, 1} * std::forward<T>(transform);
    vertices[3] = glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_max.z, 1} * std::forward<T>(transform);
    vertices[4] = glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_max.z, 1} * std::forward<T>(transform);
    vertices[5] = glm::vec4{aabb.m_max.x, aabb.m_max.y, aabb.m_min.z, 1} * std::forward<T>(transform);
    vertices[6] = glm::vec4{aabb.m_min, 1} * std::forward<T>(transform);
    vertices[7] = glm::vec4{aabb.m_max, 1} * std::forward<T>(transform);

    return AxisAlignedBoundingBox(vertices);
}

template<typename T> requires std::is_same_v<glm::mat4, std::decay_t<T>>
const AxisAlignedBoundingBox operator*(T&& transform, AxisAlignedBoundingBox&& aabb)
{
    std::vector<glm::vec3> vertices(8);
    vertices[0] = std::forward<T>(transform) * glm::vec4{aabb.m_min.x, aabb.m_min.y, aabb.m_max.z, 1};
    vertices[1] = std::forward<T>(transform) * glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_min.z, 1};
    vertices[2] = std::forward<T>(transform) * glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_min.z, 1};
    vertices[3] = std::forward<T>(transform) * glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_max.z, 1};
    vertices[4] = std::forward<T>(transform) * glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_max.z, 1};
    vertices[5] = std::forward<T>(transform) * glm::vec4{aabb.m_max.x, aabb.m_max.y, aabb.m_min.z, 1};
    vertices[6] = std::forward<T>(transform) * glm::vec4{aabb.m_min, 1};
    vertices[7] = std::forward<T>(transform) * glm::vec4{aabb.m_max, 1};

    for (auto& vertex : vertices)
    {
        aabb.m_min = glm::min(aabb.m_min, vertex);
        aabb.m_max = glm::max(aabb.m_max, vertex);
    }
    aabb.m_center = (aabb.m_min + aabb.m_max) * 0.5f;

    return std::move(aabb);
}

template<typename T> requires std::is_same_v<glm::mat4, std::decay_t<T>>
const AxisAlignedBoundingBox operator*(AxisAlignedBoundingBox&& aabb, T&& transform)
{
    std::vector<glm::vec3> vertices(8);
    vertices[0] = glm::vec4{aabb.m_min.x, aabb.m_min.y, aabb.m_max.z, 1} * std::forward<T>(transform);
    vertices[1] = glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_min.z, 1} * std::forward<T>(transform);
    vertices[2] = glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_min.z, 1} * std::forward<T>(transform);
    vertices[3] = glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_max.z, 1} * std::forward<T>(transform);
    vertices[4] = glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_max.z, 1} * std::forward<T>(transform);
    vertices[5] = glm::vec4{aabb.m_max.x, aabb.m_max.y, aabb.m_min.z, 1} * std::forward<T>(transform);
    vertices[6] = glm::vec4{aabb.m_min, 1} * std::forward<T>(transform);
    vertices[7] = glm::vec4{aabb.m_max, 1} * std::forward<T>(transform);

    for (auto& vertex : vertices)
    {
        aabb.m_min = glm::min(aabb.m_min, vertex);
        aabb.m_max = glm::max(aabb.m_max, vertex);
    }
    aabb.m_center = (aabb.m_min + aabb.m_max) * 0.5f;

    return std::move(aabb);
}
} // namespace Aurora