#pragma once
// std include
#include <vector>
// thirdparty include

// Aurora include


namespace Aurora
{
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class SubMesh
{
    friend class MeshRenderMaterial;
    friend class MeshPhongPass;
    friend class MeshOutlinePass;
public:
    SubMesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::vector<TextureInfo>&& textures)
        : m_vertices(vertices), m_indices(indices), m_textures(textures) { }

    SubMesh(SubMesh&& other)
        : m_vertices(std::move(other.m_vertices)), m_indices(std::move(other.m_indices)), m_textures(std::move(other.m_textures))
    {
        other.m_vertices.clear(); other.m_indices.clear(); other.m_textures.clear();
    }

    ~SubMesh() = default;

private:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<TextureInfo> m_textures;
};
} // namespace Aurora
