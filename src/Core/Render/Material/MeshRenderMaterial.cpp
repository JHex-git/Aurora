// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "Runtime/Scene/SubMesh.h"
#include "glWrapper/Texture.h"
#include "Runtime/Scene/Camera.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/LightManager.h"
#include "Runtime/Scene/TextureManager.h"
#include "Core/Render/RenderSystem.h"

namespace Aurora
{

void MeshRenderMaterial::Serialize(tinyxml2::XMLElement *node)
{
    node->SetName("Material");
    node->Attribute("Type", "Default");
}

void MeshRenderMaterial::Deserialize(const tinyxml2::XMLElement *node)
{
    Init();
}

bool MeshRenderMaterial::Init()
{
    m_vbos.resize(m_mesh->m_submeshes.size());
    for (size_t i = 0; i < m_mesh->m_submeshes.size(); ++i)
    {
        m_vbos[i] = std::make_shared<VertexBuffer>();
        m_vbos[i]->LoadData(m_mesh->m_submeshes[i].m_vertices, m_mesh->m_submeshes[i].m_vertices.size());
        m_vbos[i]->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)});
        m_vbos[i]->SetAttribPointer(VertexAttribPointer{1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)});
        m_vbos[i]->SetAttribPointer(VertexAttribPointer{2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords)});
    }
    
    m_ebos.resize(m_mesh->m_submeshes.size());
    for (size_t i = 0; i < m_mesh->m_submeshes.size(); ++i)
    {
        m_ebos[i] = std::make_shared<ElementBuffer>();
        m_ebos[i]->LoadData(m_mesh->m_submeshes[i].m_indices, m_mesh->m_submeshes[i].m_indices.size());
    }

    RenderSystem::GetInstance().AddMeshRenderMaterial(shared_from_this());
    return true;
}
} // namespace Aurora