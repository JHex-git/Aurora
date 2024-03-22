// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Core/Render/Pass/MeshRenderPass.h"
#include "Runtime/Scene/SubMesh.h"
#include "glWrapper/Texture.h"
#include "Runtime/Scene/Camera.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/LightManager.h"
#include "Runtime/Scene/TextureManager.h"

namespace Aurora
{

void MeshRenderPass::Serialize(tinyxml2::XMLElement *node)
{
    node->SetName("Material");
    node->Attribute("Type", "Default");
}

void MeshRenderPass::Deserialize(const tinyxml2::XMLElement *node)
{
    Init();
}

bool MeshRenderPass::Init()
{
    std::vector<Shader> shaders;
    shaders.emplace_back(ShaderType::VertexShader);
    if (!shaders[0].Load(FileSystem::GetFullPath("assets/shaders/bypass.vert")))
    {
        spdlog::error("Failed to load vertex shader {}", FileSystem::GetFullPath("assets/shaders/bypass.vert"));
        return false;
    }
    shaders.emplace_back(ShaderType::FragmentShader);
    if (!shaders[1].Load(FileSystem::GetFullPath("assets/shaders/phong.frag")))
    {
        spdlog::error("Failed to load fragment shader {}", FileSystem::GetFullPath("assets/shaders/phong.frag"));
        return false;
    }
    m_shader_program = std::make_shared<ShaderProgram>();
    if (!m_shader_program->Load(shaders))
    {
        spdlog::error("Failed to load shader program");
        return false;
    }

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

    return true;
}

void MeshRenderPass::Render()
{
    if (m_shader_program != nullptr)
    {
        m_shader_program->Bind();
        // TODO: delete this
        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
        m_shader_program->SetUniform("uModel", model);
        // m_shader_program->SetUniform("uModel", glm::identity<glm::mat4>());
        m_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
        m_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
        m_shader_program->SetUniform("uViewPos", MainCamera::GetInstance().GetPosition());
        const auto& light = LightManager::GetInstance().GetLight(MainCamera::GetInstance().GetPosition());
        m_shader_program->SetUniform("uLightPos", light.GetPosition());
        m_shader_program->SetUniform("uLightColor", light.GetColor());
        for (size_t i = 0; i < m_mesh->m_submeshes.size(); ++i)
        {
            m_vbos[i]->Bind();
            m_ebos[i]->Bind();
            for (int j = 0; j < m_mesh->m_submeshes[i].m_textures.size(); ++j)
            {
                auto texture = TextureManager::GetInstance().GetTexture(m_mesh->m_submeshes[i].m_textures[j].m_filePath);
                if (texture)
                {
                    // reserve unit 0 for temporary use
                    texture->Bind(j + 1);
                    m_shader_program->SetUniform(std::string("uTex") + m_mesh->m_submeshes[i].m_textures[j].m_type, j + 1);
                }
            }
            glDrawElements(GL_TRIANGLES, m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
            m_ebos[i]->Unbind();
            m_vbos[i]->Unbind();
        }
        m_shader_program->Unbind();
    }
}
} // namespace Aurora