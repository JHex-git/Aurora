// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/MeshPhongPass.h"
#include "Utility/FileSystem.h"
#include "glWrapper/Shader.h"
#include "glWrapper/RenderEventInfo.h"
#include "Runtime/Scene/Camera.h"
#include "Runtime/Scene/LightManager.h"
#include "Runtime/Scene/TextureManager.h"
#include "glWrapper/Utils.h"

namespace Aurora
{

namespace
{
constexpr std::array<glm::vec3, 6> k_dirs = {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), 
                                           glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), 
                                           glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f)};
constexpr std::array<glm::vec3, 6> k_ups = {glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -1.f, 0.f),
                                            glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, -1.f),
                                            glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -1.f, 0.f)};
}

bool MeshPhongPass::Init(const std::array<int, 2>& viewport_size)
{
    if (!RenderPass::Init(viewport_size)) return false;
    
    auto fbo = FrameBufferObjectBuilder(viewport_size[0], viewport_size[1])
                                        .AddColorAttachment({.internal_format = GL_RGBA})
                                        .EnableDepthAttachment({}).Create();
    if (!fbo.has_value()) return false;
    m_fbo = std::make_shared<FrameBufferObject>(std::move(fbo.value()));

    auto shading_fbo = FrameBufferObjectBuilder(viewport_size[0], viewport_size[1])
                                        .AddColorAttachment({.internal_format = GL_RGBA})
                                        .EnableDepthAttachment({}).Create();
    if (!shading_fbo.has_value()) return false;
    m_shading_fbo = std::make_shared<FrameBufferObject>(std::move(shading_fbo.value()));

    auto shadow_map_fbo = FrameBufferObjectBuilder(1024, 1024).EnableDepthAttachment({.type = Texture::Type::Cubemap}).Create();
    if (!shadow_map_fbo.has_value()) return false;
    m_shadow_map_fbo = std::make_shared<FrameBufferObject>(std::move(shadow_map_fbo.value()));

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

    // shadow map shader program
    {
        std::vector<Shader> shaders;
        shaders.emplace_back(ShaderType::VertexShader);
        auto vert_path = FileSystem::GetFullPath("shaders/mesh.vert");
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
        m_shadow_map_shader_program = std::make_unique<ShaderProgram>();
        if (!m_shadow_map_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    // composite shader program
    {
        std::vector<Shader> shaders;
        shaders.emplace_back(ShaderType::VertexShader);
        auto vert_path = FileSystem::GetFullPath("shaders/quad.vert");
        if (!shaders[0].Load(vert_path))
        {
            spdlog::error("Failed to load vertex shader {}", vert_path);
            return false;
        }
        shaders.emplace_back(ShaderType::FragmentShader);
        auto frag_path = FileSystem::GetFullPath("shaders/copy.frag");
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_composite_shader_program = std::make_unique<ShaderProgram>();
        if (!m_composite_shader_program->Load(shaders))
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (m_tex_shader_program != nullptr && m_no_tex_shader_program != nullptr)
    {
        glEnable(GL_DEPTH_TEST);

        SCOPED_RENDER_EVENT("MeshPhongPass");

        const auto& lights = LightManager::GetInstance().GetActiveLights();
        for (auto p_light : lights)
        {
            auto light = p_light.lock();
            if (!light) continue;

            const auto light_owner = light->GetOwner().lock();
            SCOPED_RENDER_EVENT(light_owner ? light_owner->GetName() : "light");

            m_shadow_map_fbo->Bind();
            for (auto i = 0; i < k_dirs.size(); ++i)
            {
                m_shadow_map_fbo->BindDepthCubemapFace(i);
                glClear(GL_DEPTH_BUFFER_BIT);
            }

            m_shading_fbo->Bind();
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            // Generate shadow map
            {
                SCOPED_RENDER_EVENT("Shadow Pass");

                m_shadow_map_fbo->Bind();
                m_shadow_map_shader_program->Bind();
                m_shadow_map_shader_program->SetUniform("uProjection", 
                    glm::perspective(glm::radians(90.f), 1.f, MainCamera::GetInstance().GetNearPlane(), MainCamera::GetInstance().GetFarPlane()));
                for (auto& p_material : m_mesh_render_materials)
                {
                    auto material = p_material.second.lock();
                    if (!material) continue;
                    
                    const auto material_owner = material->GetOwner().lock();
                    SCOPED_RENDER_EVENT(material_owner ? material_owner->GetName() : "mesh render material");

                    const glm::mat4 model = material->GetModelMatrix();
                    m_shadow_map_shader_program->SetUniform("uModel", model);

                    for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
                    {
                        material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)});
                        material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)});
                        if (material->m_mesh->HasTextures())
                            material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords)});

                        material->m_vbos[i]->Bind();
                        material->m_ebos[i]->Bind();
                        // render shadow map for each face of the cubemap
                        for (auto j = 0; j < k_dirs.size(); ++j)
                        {
                            m_shadow_map_shader_program->SetUniform("uView", glm::lookAt(light->GetPosition(), light->GetPosition() + k_dirs[j], k_ups[j]));
                            m_shadow_map_fbo->BindDepthCubemapFace(j);
                            glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                        }
                        material->m_ebos[i]->Unbind();
                        material->m_vbos[i]->Unbind();
                    }
                }
            }

            // Shading pass
            {
                SCOPED_RENDER_EVENT("Shading Pass");

                m_shading_fbo->Bind();
                for (auto& p_material : m_mesh_render_materials)
                {
                    auto material = p_material.second.lock();
                    if (!material) continue;

                    auto mesh_shader_program = material->m_mesh->HasTextures() ? m_tex_shader_program.get() : m_no_tex_shader_program.get();
                    mesh_shader_program->Bind();
                    const glm::mat4 model = material->GetModelMatrix();
                    mesh_shader_program->SetUniform("uModel", model);
                    mesh_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
                    mesh_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
                    mesh_shader_program->SetUniform("uViewPos", MainCamera::GetInstance().GetPosition());
                    mesh_shader_program->SetUniform("uLightPos", light->GetPosition());
                    mesh_shader_program->SetUniform("uLightColor", light->GetColor() * light->GetIntensity());
                    mesh_shader_program->SetUniform("uZNear", MainCamera::GetInstance().GetNearPlane());
                    mesh_shader_program->SetUniform("uZFar", MainCamera::GetInstance().GetFarPlane());

                    for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
                    {
                        material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)});
                        material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)});
                        if (material->m_mesh->HasTextures())
                            material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords)});

                        material->m_vbos[i]->Bind();
                        material->m_ebos[i]->Bind();

                        int current_unit = 1;
                        if (material->m_mesh->HasTextures())
                        {
                            for (int j = 0; j < material->m_mesh->m_submeshes[i].m_textures.size(); ++j)
                            {
                                auto& surface_texture = TextureManager::GetInstance().GetTexture(material->m_mesh->m_submeshes[i].m_textures[j]);
                                // reserve unit 0 for temporary use
                                surface_texture.texture.Bind(current_unit++);
                                mesh_shader_program->SetUniform(std::string("uTex") + surface_texture.type, current_unit - 1);
                            }
                        }
                        else
                        {
                            mesh_shader_program->SetUniform("uColor", material->m_mesh->m_submeshes[i].m_color);
                        }

                        // use shadow map to decide the light contribution
                        m_shadow_map_fbo->BindDepthTexture(current_unit++);
                        mesh_shader_program->SetUniform("uTexShadowMap", current_unit - 1);
                        glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                        material->m_ebos[i]->Unbind();
                        material->m_vbos[i]->Unbind();
                    }
                    mesh_shader_program->Unbind();
                }
            }

            {
                SCOPED_RENDER_EVENT("Composite Pass");

                m_fbo->Bind();
                glDisable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE);
                glBlendEquation(GL_FUNC_ADD);
                m_shading_fbo->BindColorTexture(0, 1);
                m_composite_shader_program->Bind();
                m_composite_shader_program->SetUniform("uTexture", 1);
                DrawQuad(1);
                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST);
            }
        }
    }

    BlitDepth(m_shading_fbo, m_fbo);
}

MeshRenderMaterialID MeshPhongPass::RegisterMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material)
{
    static MeshRenderMaterialID id = 1;
    m_mesh_render_materials.emplace(id, mesh_render_material);
    return id++;
}

void MeshPhongPass::UnregisterMeshRenderMaterial(MeshRenderMaterialID id)
{
    m_mesh_render_materials.erase(id);
}
} // namespace Aurora