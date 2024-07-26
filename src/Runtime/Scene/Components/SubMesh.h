#pragma once
// std include
#include <vector>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
// Aurora include
#include "glWrapper/Texture.h"

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
    friend class SkyboxRenderMaterial;
    friend class MeshPhongPass;
    friend class MeshOutlinePass;
    friend class SkyboxPass;
public:
    SubMesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::vector<TextureID>&& textures)
        : m_vertices(std::move(vertices)), m_indices(std::move(indices)), m_textures(std::move(textures)) { }

    SubMesh(SubMesh&& other)
        : m_vertices(std::move(other.m_vertices)), m_indices(std::move(other.m_indices)), m_textures(std::move(other.m_textures))
    {
        other.m_vertices.clear(); other.m_indices.clear(); other.m_textures.clear();
    }

    ~SubMesh() = default;

private:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<TextureID> m_textures;
};
} // namespace Aurora
