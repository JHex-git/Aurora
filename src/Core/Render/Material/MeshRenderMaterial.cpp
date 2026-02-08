// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "Runtime/Scene/Components/SubMesh.h"
#include "glWrapper/Texture.h"
#include "Runtime/Scene/Camera.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/LightManager.h"
#include "Core/Render/RenderSystem.h"
#include "Runtime/Scene/SceneManager.h"

namespace Aurora
{

void MeshRenderMaterial::Serialize(tinyxml2::XMLElement *node)
{
    node->SetName("Material");
    node->SetAttribute("Type", "Default");
}

void MeshRenderMaterial::Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner)
{
    Init(owner);
}

bool MeshRenderMaterial::Init(std::shared_ptr<SceneObject> owner)
{
    RenderMaterial::Init(owner);

    m_vbos.resize(m_mesh->m_submeshes.size());
    for (size_t i = 0; i < m_mesh->m_submeshes.size(); ++i)
    {
        m_vbos[i] = std::make_shared<VertexBuffer>();
        m_vbos[i]->LoadData(m_mesh->m_submeshes[i].m_vertices, m_mesh->m_submeshes[i].m_vertices.size());
    }
    
    m_ebos.resize(m_mesh->m_submeshes.size());
    for (size_t i = 0; i < m_mesh->m_submeshes.size(); ++i)
    {
        m_ebos[i] = std::make_shared<ElementBuffer>();
        m_ebos[i]->LoadData(m_mesh->m_submeshes[i].m_indices, m_mesh->m_submeshes[i].m_indices.size());
    }

    m_vaos.resize(m_mesh->m_submeshes.size());
    for (size_t i = 0; i < m_mesh->m_submeshes.size(); ++i)
    {
        m_vaos[i] = std::make_shared<VertexArray>();
        m_vaos[i]->Bind();
        m_vbos[i]->Bind();
        m_ebos[i]->Bind();
        m_vaos[i]->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)});
        m_vaos[i]->SetAttribPointer(VertexAttribPointer{1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)});
        if (m_mesh->HasTextures())
            m_vaos[i]->SetAttribPointer(VertexAttribPointer{2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords)});
        m_vaos[i]->Unbind();
    }
    
    return true;
}

const glm::mat4 MeshRenderMaterial::GetModelMatrix() const
{
    auto scene_object = m_scene_object.lock();
    if (!scene_object) return glm::identity<glm::mat4>();
    auto transform = scene_object->GetTransform();
    return *transform;
}
} // namespace Aurora