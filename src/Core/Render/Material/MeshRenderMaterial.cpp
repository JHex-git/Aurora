// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
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

const glm::mat4 MeshRenderMaterial::GetModelMatrix() const
{
    auto transform = m_scene_object.lock()->GetTransform();

    glm::vec3 position = transform->GetField<glm::vec3>("m_position");
    glm::quat rotation = transform->GetField<glm::quat>("m_rotation");
    glm::vec3 scale = transform->GetField<glm::vec3>("m_scale");
    glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), position) * glm::mat4_cast(rotation) * glm::scale(glm::identity<glm::mat4>(), scale);
    return model;
}
} // namespace Aurora