// std include

// thirdparty include

// Aurora include
#include "Math/AxisAlignedBoundingBox.h"

namespace Aurora
{

AxisAlignedBoundingBox::AxisAlignedBoundingBox(const std::vector<glm::vec3>& vertices)
{
    for (const auto& vertex : vertices)
    {
        m_min = glm::min(m_min, vertex);
        m_max = glm::max(m_max, vertex);
    }
    m_center = (m_min + m_max) * 0.5f;
}

AxisAlignedBoundingBox::AxisAlignedBoundingBox(const glm::vec3& min, const glm::vec3& max) : 
    m_min(min), m_max(max), m_center((min + max) * 0.5f) { }

AxisAlignedBoundingBox& AxisAlignedBoundingBox::operator|=(const AxisAlignedBoundingBox& aabb)
{
    m_min = glm::min(m_min, aabb.GetMin());
    m_max = glm::max(m_max, aabb.GetMax());
    m_center = (m_min + m_max) * 0.5f;
    return *this;
}

const AxisAlignedBoundingBox AxisAlignedBoundingBox::operator|(const AxisAlignedBoundingBox& aabb) const
{
    AxisAlignedBoundingBox result = *this;
    result |= aabb;
    return result;
}

const AxisAlignedBoundingBox AxisAlignedBoundingBox::operator|(AxisAlignedBoundingBox&& aabb) const
{
    aabb |= *this;
    return std::move(aabb);
}

AxisAlignedBoundingBox& AxisAlignedBoundingBox::operator&=(const AxisAlignedBoundingBox& aabb)
{
    m_min = glm::max(m_min, aabb.GetMin());
    m_max = glm::min(m_max, aabb.GetMax());
    m_center = (m_min + m_max) * 0.5f;
    return *this;
}

const AxisAlignedBoundingBox AxisAlignedBoundingBox::operator&(const AxisAlignedBoundingBox& aabb) const
{
    AxisAlignedBoundingBox result = *this;
    result &= aabb;
    return result;
}

const AxisAlignedBoundingBox AxisAlignedBoundingBox::operator&(AxisAlignedBoundingBox&& aabb) const
{
    aabb &= *this;
    return std::move(aabb);
}

// const AxisAlignedBoundingBox operator*(const glm::mat4& transform, const AxisAlignedBoundingBox& aabb)
// {
//     std::vector<glm::vec3> vertices(8);
//     vertices[0] = transform * glm::vec4{aabb.m_min.x, aabb.m_min.y, aabb.m_max.z, 1};
//     vertices[1] = transform * glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_min.z, 1};
//     vertices[2] = transform * glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_min.z, 1};
//     vertices[3] = transform * glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_max.z, 1};
//     vertices[4] = transform * glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_max.z, 1};
//     vertices[5] = transform * glm::vec4{aabb.m_max.x, aabb.m_max.y, aabb.m_min.z, 1};
//     vertices[6] = transform * glm::vec4{aabb.m_min, 1};
//     vertices[7] = transform * glm::vec4{aabb.m_max, 1};

//     return AxisAlignedBoundingBox(vertices);
// }

// const AxisAlignedBoundingBox operator*(const AxisAlignedBoundingBox& aabb, const glm::mat4& transform)
// {
//     std::vector<glm::vec3> vertices(8);
//     vertices[0] = glm::vec4{aabb.m_min.x, aabb.m_min.y, aabb.m_max.z, 1} * transform;
//     vertices[1] = glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_min.z, 1} * transform;
//     vertices[2] = glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_min.z, 1} * transform;
//     vertices[3] = glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_max.z, 1} * transform;
//     vertices[4] = glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_max.z, 1} * transform;
//     vertices[5] = glm::vec4{aabb.m_max.x, aabb.m_max.y, aabb.m_min.z, 1} * transform;
//     vertices[6] = glm::vec4{aabb.m_min, 1} * transform;
//     vertices[7] = glm::vec4{aabb.m_max, 1} * transform;

//     return AxisAlignedBoundingBox(vertices);
// }

// const AxisAlignedBoundingBox operator*(const glm::mat4& transform, AxisAlignedBoundingBox&& aabb)
// {
//     std::vector<glm::vec3> vertices(8);
//     vertices[0] = transform * glm::vec4{aabb.m_min.x, aabb.m_min.y, aabb.m_max.z, 1};
//     vertices[1] = transform * glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_min.z, 1};
//     vertices[2] = transform * glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_min.z, 1};
//     vertices[3] = transform * glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_max.z, 1};
//     vertices[4] = transform * glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_max.z, 1};
//     vertices[5] = transform * glm::vec4{aabb.m_max.x, aabb.m_max.y, aabb.m_min.z, 1};
//     vertices[6] = transform * glm::vec4{aabb.m_min, 1};
//     vertices[7] = transform * glm::vec4{aabb.m_max, 1};

//     for (auto& vertex : vertices)
//     {
//         aabb.m_min = glm::min(aabb.m_min, vertex);
//         aabb.m_max = glm::max(aabb.m_max, vertex);
//     }
//     aabb.m_center = (aabb.m_min + aabb.m_max) * 0.5f;

//     return std::move(aabb);
// }

// const AxisAlignedBoundingBox operator*(AxisAlignedBoundingBox&& aabb, const glm::mat4& transform)
// {
//     std::vector<glm::vec3> vertices(8);
//     vertices[0] = glm::vec4{aabb.m_min.x, aabb.m_min.y, aabb.m_max.z, 1} * transform;
//     vertices[1] = glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_min.z, 1} * transform;
//     vertices[2] = glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_min.z, 1} * transform;
//     vertices[3] = glm::vec4{aabb.m_min.x, aabb.m_max.y, aabb.m_max.z, 1} * transform;
//     vertices[4] = glm::vec4{aabb.m_max.x, aabb.m_min.y, aabb.m_max.z, 1} * transform;
//     vertices[5] = glm::vec4{aabb.m_max.x, aabb.m_max.y, aabb.m_min.z, 1} * transform;
//     vertices[6] = glm::vec4{aabb.m_min, 1} * transform;
//     vertices[7] = glm::vec4{aabb.m_max, 1} * transform;

//     for (auto& vertex : vertices)
//     {
//         aabb.m_min = glm::min(aabb.m_min, vertex);
//         aabb.m_max = glm::max(aabb.m_max, vertex);
//     }
//     aabb.m_center = (aabb.m_min + aabb.m_max) * 0.5f;

//     return std::move(aabb);
// }

// const AxisAlignedBoundingBox operator*(glm::mat4&& transform, const AxisAlignedBoundingBox& aabb);
// const AxisAlignedBoundingBox operator*(const AxisAlignedBoundingBox& aabb, glm::mat4&& transform);
// const AxisAlignedBoundingBox operator*(glm::mat4&& transform, AxisAlignedBoundingBox&& aabb);
// const AxisAlignedBoundingBox operator*(AxisAlignedBoundingBox&& aabb, glm::mat4&& transform);

const bool AxisAlignedBoundingBox::IsInside(const AxisAlignedBoundingBox& aabb) const
{
    return m_min.x >= aabb.m_min.x && m_min.y >= aabb.m_min.y && m_min.z >= aabb.m_min.z &&
           m_max.x <= aabb.m_max.x && m_max.y <= aabb.m_max.y && m_max.z <= aabb.m_max.z;
}
} // namespace Aurora