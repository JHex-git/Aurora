// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/MeshPhongPass.h"
#include "Utility/FileSystem.h"
#include "glWrapper/Shader.h"
#include "Runtime/Scene/Camera.h"
#include "Runtime/Scene/LightManager.h"
#include "Runtime/Scene/TextureManager.h"

namespace Aurora
{

bool MeshPhongPass::Init()
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
    m_shader_program = std::make_unique<ShaderProgram>();
    if (!m_shader_program->Load(shaders))
    {
        spdlog::error("Failed to load shader program");
        return false;
    }

    return true;
}

void MeshPhongPass::Render()
{
    if (m_shader_program != nullptr)
    {
        glEnable(GL_DEPTH_TEST);
        m_shader_program->Bind();
        for (auto& material : m_mesh_render_materials)
        {
            // render the loaded model
            const glm::mat4 model = material->GetModelMatrix();
            m_shader_program->SetUniform("uModel", model);
            m_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
            m_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
            m_shader_program->SetUniform("uViewPos", MainCamera::GetInstance().GetPosition());
            const auto& light = LightManager::GetInstance().GetLight(MainCamera::GetInstance().GetPosition());
            m_shader_program->SetUniform("uLightPos", light.GetPosition());
            m_shader_program->SetUniform("uLightColor", light.GetColor());
            for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
            {
                material->m_vbos[i]->Bind();
                material->m_ebos[i]->Bind();
                for (int j = 0; j < material->m_mesh->m_submeshes[i].m_textures.size(); ++j)
                {
                    auto texture = TextureManager::GetInstance().GetTexture(material->m_mesh->m_submeshes[i].m_textures[j].m_filePath);
                    if (texture)
                    {
                        // reserve unit 0 for temporary use
                        texture->Bind(j + 1);
                        m_shader_program->SetUniform(std::string("uTex") + material->m_mesh->m_submeshes[i].m_textures[j].m_type, j + 1);
                    }
                }
                glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                material->m_ebos[i]->Unbind();
                material->m_vbos[i]->Unbind();
            }
        }
        m_shader_program->Unbind();
    }
}
} // namespace Aurora