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

bool MeshPhongPass::Init(const std::array<int, 2>& viewport_size)
{
    if (!RenderPass::Init(viewport_size)) return false;
    
    auto fbo = FrameBufferObjectBuilder(viewport_size[0], viewport_size[1])
                                        .AddColorAttachment(GL_RGBA)
                                        .EnableDepthAttachmentOnly().Create();
    if (!fbo.has_value()) return false;
    m_fbo = std::make_shared<FrameBufferObject>(std::move(fbo.value()));

    // shader program with texture
    {
        std::vector<Shader> shaders;
        shaders.emplace_back(ShaderType::VertexShader);
        auto vert_path = FileSystem::GetFullPath("shaders/mesh.vert");
        shaders[0].SetFlag("ENABLE_NORMALS");
        shaders[0].SetFlag("ENABLE_TEXCOORDS");
        if (!shaders[0].Load(vert_path))
        {
            spdlog::error("Failed to load vertex shader {}", vert_path);
            return false;
        }
        shaders.emplace_back(ShaderType::FragmentShader);
        auto frag_path = FileSystem::GetFullPath("shaders/mesh_phong.frag");
        shaders[1].SetFlag("ENABLE_TEXCOORDS");
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_tex_shader_program = std::make_unique<ShaderProgram>();
        if (!m_tex_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    // shader program without texture
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
        auto frag_path = FileSystem::GetFullPath("shaders/mesh_phong.frag");
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_no_tex_shader_program = std::make_unique<ShaderProgram>();
        if (!m_no_tex_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    return true;
}

void MeshPhongPass::Render()
{
    m_fbo->Bind();
    glViewport(0, 0, m_viewport_size[0], m_viewport_size[1]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (m_tex_shader_program != nullptr && m_no_tex_shader_program != nullptr)
    {
        glEnable(GL_DEPTH_TEST);
        const auto& lights = LightManager::GetInstance().GetActiveLights();
        for (auto light : lights)
        {
            for (auto& material : m_mesh_render_materials)
            {
                auto shader_program = material->m_mesh->HasTextures() ? m_tex_shader_program.get() : m_no_tex_shader_program.get();
                shader_program->Bind();
                const glm::mat4 model = material->GetModelMatrix();
                shader_program->SetUniform("uModel", model);
                shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
                shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
                shader_program->SetUniform("uViewPos", MainCamera::GetInstance().GetPosition());
                shader_program->SetUniform("uLightPos", light->GetPosition());
                shader_program->SetUniform("uLightColor", light->GetColor());
                for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
                {
                    material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)});
                    material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)});
                    if (material->m_mesh->HasTextures())
                        material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords)});

                    material->m_vbos[i]->Bind();
                    material->m_ebos[i]->Bind();
                    if (material->m_mesh->HasTextures())
                    {
                        for (int j = 0; j < material->m_mesh->m_submeshes[i].m_textures.size(); ++j)
                        {
                            auto& surface_texture = TextureManager::GetInstance().GetTexture(material->m_mesh->m_submeshes[i].m_textures[j]);
                            // reserve unit 0 for temporary use
                            surface_texture.texture.Bind(j + 1);
                            shader_program->SetUniform(std::string("uTex") + surface_texture.type, j + 1);
                        }
                    }
                    else
                    {
                        shader_program->SetUniform("uColor", material->m_mesh->m_submeshes[i].m_color);
                    }
                    glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                    material->m_ebos[i]->Unbind();
                    material->m_vbos[i]->Unbind();
                }
                shader_program->Unbind();
            }
        }
    }
    m_fbo->Unbind();
}
} // namespace Aurora