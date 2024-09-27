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
    glm::vec2 tex_coords;
    glm::vec4 color;
};

class SubMesh
{
    friend class MeshRenderMaterial;
    friend class SkyboxRenderMaterial;
    friend class ForwardRenderPass;
    friend class MeshOutlinePass;
    friend class SkyboxPass;
    friend class GizmosPass;
public:
    SubMesh(aiColor3D color, std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::vector<TextureID>&& textures)
        : m_color(color.r, color.g, color.b), m_vertices(std::move(vertices)), m_indices(std::move(indices)), m_textures(std::move(textures)) { }

    SubMesh(SubMesh&& other) = default;

    ~SubMesh() = default;

private:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<TextureID> m_textures;

    glm::vec3 m_color;
};
} // namespace Aurora
