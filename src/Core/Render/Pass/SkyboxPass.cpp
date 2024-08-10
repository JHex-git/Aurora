// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/SkyboxPass.h"
#include "Utility/FileSystem.h"
#include "glWrapper/Shader.h"
#include "Runtime/Scene/Camera.h"

namespace Aurora
{

constexpr float outline_thickness = 10.f;
bool SkyboxPass::Init(const std::array<int, 2>& viewport_size)
{
    if (!RenderPass::Init(viewport_size)) return false;
    
    auto fbo = FrameBufferObjectBuilder(viewport_size[0], viewport_size[1])
                                        .AddColorAttachment()
                                        .EnableDepthAttachmentOnly().Create();
    if (!fbo.has_value()) return false;
    m_fbo = std::make_shared<FrameBufferObject>(std::move(fbo.value()));

    std::vector<Shader> shaders;
    shaders.emplace_back(ShaderType::VertexShader);
    if (!shaders[0].Load(FileSystem::GetFullPath("shaders/skybox.vert")))
    {
        spdlog::error("Failed to load vertex shader {}", FileSystem::GetFullPath("shaders/skybox.vert"));
        return false;
    }
    shaders.emplace_back(ShaderType::FragmentShader);
    if (!shaders[1].Load(FileSystem::GetFullPath("shaders/skybox.frag")))
    {
        spdlog::error("Failed to load fragment shader {}", FileSystem::GetFullPath("shaders/skybox.frag"));
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

void SkyboxPass::Render()
{
    m_fbo->Bind();
    glViewport(0, 0, m_viewport_size[0], m_viewport_size[1]);

    if (m_shader_program != nullptr && m_skybox_render_material != nullptr)
    {
        glDepthFunc(GL_LEQUAL);
        glCullFace(GL_FRONT);
        m_shader_program->Bind();
        // render the loaded model
        const glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), MainCamera::GetInstance().GetPosition());
        m_shader_program->SetUniform("uMVP", MainCamera::GetInstance().GetProjectionMatrix() * MainCamera::GetInstance().GetViewMatrix() * model);
        for (size_t i = 0; i < m_skybox_render_material->m_mesh->m_submeshes.size(); ++i)
        {
            m_skybox_render_material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)});
            
            m_skybox_render_material->m_vbos[i]->Bind();
            m_skybox_render_material->m_ebos[i]->Bind();
            m_skybox_render_material->GetCubemapTexture()->Bind();
            glDrawElements(GL_TRIANGLES, m_skybox_render_material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
            m_skybox_render_material->m_ebos[i]->Unbind();
            m_skybox_render_material->m_vbos[i]->Unbind();
        }
        glDepthFunc(GL_LESS);
        glCullFace(GL_BACK);
        m_shader_program->Unbind();
    }
    m_fbo->Unbind();
}
} // namespace Aurora