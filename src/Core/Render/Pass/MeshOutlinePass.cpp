// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/MeshOutlinePass.h"
#include "Utility/FileSystem.h"
#include "glWrapper/Shader.h"
#include "Runtime/Scene/Camera.h"

namespace Aurora
{

constexpr float outline_thickness = 10.f;
bool MeshOutlinePass::Init()
{
    std::vector<Shader> shaders;
    shaders.emplace_back(ShaderType::VertexShader);
    if (!shaders[0].Load(FileSystem::GetFullPath("shaders/outline.vert")))
    {
        spdlog::error("Failed to load vertex shader {}", FileSystem::GetFullPath("shaders/outline.vert"));
        return false;
    }
    shaders.emplace_back(ShaderType::FragmentShader);
    if (!shaders[1].Load(FileSystem::GetFullPath("shaders/pure.frag")))
    {
        spdlog::error("Failed to load fragment shader {}", FileSystem::GetFullPath("shaders/pure.frag"));
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

void MeshOutlinePass::Render(const std::array<int, 2>& viewport_size)
{
    if (m_shader_program != nullptr)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glCullFace(GL_FRONT);
        m_shader_program->Bind();
        // render the loaded model
        const glm::mat4 model = m_selected_mesh_render_material->GetModelMatrix();
        m_shader_program->SetUniform("uModel", model);
        m_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
        m_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
        m_shader_program->SetUniform("uOutlineThickness", outline_thickness);
        m_shader_program->SetUniform("uScreenWidth", viewport_size[0]);
        m_shader_program->SetUniform("uScreenHeight", viewport_size[1]);
        m_shader_program->SetUniform("uColor", glm::vec3(1.0f, 1.0f, 1.0f));
        for (size_t i = 0; i < m_selected_mesh_render_material->m_mesh->m_submeshes.size(); ++i)
        {
            m_selected_mesh_render_material->m_vbos[i]->Bind();
            m_selected_mesh_render_material->m_ebos[i]->Bind();
            glDrawElements(GL_TRIANGLES, m_selected_mesh_render_material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
            m_selected_mesh_render_material->m_ebos[i]->Unbind();
            m_selected_mesh_render_material->m_vbos[i]->Unbind();
        }
        glDepthMask(GL_TRUE);
        glCullFace(GL_BACK);
        m_shader_program->Unbind();
    }
}
} // namespace Aurora