// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/MeshOutlinePass.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/Camera.h"

// Stencil Buffer 1 means selected mesh
namespace Aurora
{

constexpr float outline_thickness = 10.f;
bool MeshOutlinePass::Init()
{
    {
        std::vector<Shader> shaders;
        shaders.emplace_back(ShaderType::VertexShader);
        auto vert_path = FileSystem::GetFullPath("shaders/mesh.vert");
        shaders[0].SetFlag("ENABLE_NORMALS");
        if (!shaders[0].Load(vert_path))
        {
            spdlog::error("Failed to load vertex shader {}", vert_path);
            return false;
        }
        shaders.emplace_back(ShaderType::FragmentShader);
        auto frag_path = FileSystem::GetFullPath("shaders/pure.frag");
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_mesh_stencil_shader_program = std::make_unique<ShaderProgram>();
        if (!m_mesh_stencil_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    {
        std::vector<Shader> shaders;
        shaders.emplace_back(ShaderType::VertexShader);
        auto vert_path = FileSystem::GetFullPath("shaders/outline.vert");
        if (!shaders[0].Load(vert_path))
        {
            spdlog::error("Failed to load vertex shader {}", vert_path);
            return false;
        }
        shaders.emplace_back(ShaderType::FragmentShader);
        auto frag_path = FileSystem::GetFullPath("shaders/pure.frag");
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_outline_shader_program = std::make_unique<ShaderProgram>();
        if (!m_outline_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    return true;
}

void MeshOutlinePass::Render(const std::array<int, 2>& viewport_size)
{
    if (m_mesh_stencil_shader_program == nullptr || m_outline_shader_program == nullptr) return;

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // disable color writing

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF); // enable writing to the stencil buffer
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // write 1 to the stencil buffer where the mesh is rendered

    // render selected model to write stencil buffer
    m_mesh_stencil_shader_program->Bind();
    const glm::mat4 model = m_selected_mesh_render_material->GetModelMatrix();
    m_mesh_stencil_shader_program->SetUniform("uModel", model);
    m_mesh_stencil_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
    m_mesh_stencil_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
    for (size_t i = 0; i < m_selected_mesh_render_material->m_mesh->m_submeshes.size(); ++i)
    {
        m_selected_mesh_render_material->m_vbos[i]->Bind();
        m_selected_mesh_render_material->m_ebos[i]->Bind();
        glDrawElements(GL_TRIANGLES, m_selected_mesh_render_material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
        m_selected_mesh_render_material->m_ebos[i]->Unbind();
        m_selected_mesh_render_material->m_vbos[i]->Unbind();
    }
    m_mesh_stencil_shader_program->Unbind();

    glDepthMask(GL_TRUE);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // enable color writing
    glCullFace(GL_FRONT);
    glStencilMask(0x00); // disable writing to the stencil buffer
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // render dilated model and use stencil buffer to render outline
    m_outline_shader_program->Bind();
    m_outline_shader_program->SetUniform("uModel", model);
    m_outline_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
    m_outline_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
    m_outline_shader_program->SetUniform("uOutlineThickness", outline_thickness);
    m_outline_shader_program->SetUniform("uScreenWidth", viewport_size[0]);
    m_outline_shader_program->SetUniform("uScreenHeight", viewport_size[1]);
    m_outline_shader_program->SetUniform("uColor", glm::vec3(1.0f, 1.0f, 1.0f));
    for (size_t i = 0; i < m_selected_mesh_render_material->m_mesh->m_submeshes.size(); ++i)
    {
        m_selected_mesh_render_material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)});
        m_selected_mesh_render_material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)});

        m_selected_mesh_render_material->m_vbos[i]->Bind();
        m_selected_mesh_render_material->m_ebos[i]->Bind();
        glDrawElements(GL_TRIANGLES, m_selected_mesh_render_material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
        m_selected_mesh_render_material->m_ebos[i]->Unbind();
        m_selected_mesh_render_material->m_vbos[i]->Unbind();
    }
    m_outline_shader_program->Unbind();

    // reset
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);
    glDisable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}
} // namespace Aurora