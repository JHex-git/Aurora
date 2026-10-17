// std include
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/DeferredRenderPass.h"
#include "Core/Render/LightUBO.h"
#include "Core/Render/PbrUtils.h"
#include "Core/Render/RenderGraph/RenderGraphBuilder.h"
#include "Core/Render/RenderGraph/RenderGraphContext.h"
#include "Utility/FileSystem.h"
#include "glWrapper/FrameBufferObject.h"
#include "glWrapper/RenderEventInfo.h"
#include "Runtime/Scene/Camera.h"
#include "Runtime/Scene/LightManager.h"
#include "Runtime/Scene/TextureManager.h"
#include "Runtime/Scene/SceneManager.h"
#include "Runtime/GlobalContext.h"
#include "glWrapper/Utils.h"
#include "glWrapper/UniformBuffer.h"

namespace Aurora
{

namespace
{
struct DeferredGeometryParameter : PassParameterBase
{
    RenderGraphTextureHandle position;
    RenderGraphTextureHandle normal;
    RenderGraphTextureHandle albedo;
    RenderGraphTextureHandle mra;
    RenderGraphTextureHandle emissive;
    RenderGraphTextureHandle depth;
};

struct DeferredLightingParameter : PassParameterBase
{
    RenderGraphTextureHandle position;
    RenderGraphTextureHandle normal;
    RenderGraphTextureHandle albedo;
    RenderGraphTextureHandle mra;
    RenderGraphTextureHandle emissive;
    RenderGraphTextureHandle color;
    ShadowPass::Result shadows;
};

} // namespace

bool DeferredRenderPass::Init()
{
    m_light_ubo = std::make_unique<UniformBuffer>(sizeof(LightUBOData), 0, GL_STATIC_DRAW);

    // G-buffer shader program with textures
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
        auto frag_path = FileSystem::GetFullPath("shaders/deferred_gbuffer.frag");
        shaders[1].SetFlag("ENABLE_TEXCOORDS");
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_gbuffer_tex_shader_program = std::make_unique<ShaderProgram>();
        if (!m_gbuffer_tex_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    // G-buffer shader program without textures
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
        auto frag_path = FileSystem::GetFullPath("shaders/deferred_gbuffer.frag");
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_gbuffer_no_tex_shader_program = std::make_unique<ShaderProgram>();
        if (!m_gbuffer_no_tex_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    // lighting shader program
    {
        std::vector<Shader> shaders;
        shaders.emplace_back(ShaderType::VertexShader);
        auto vert_path = FileSystem::GetFullPath("shaders/tonemap.vert");
        if (!shaders[0].Load(vert_path))
        {
            spdlog::error("Failed to load vertex shader {}", vert_path);
            return false;
        }
        shaders.emplace_back(ShaderType::FragmentShader);
        auto frag_path = FileSystem::GetFullPath("shaders/deferred_lighting.frag");
        shaders[1].SetOption("DIRECTIONAL_CASCADE_COUNT", kDirectionalCascadeMax);
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_lighting_shader_program = std::make_unique<ShaderProgram>();
        if (!m_lighting_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    LightManager::GetInstance().AddLightEventListener(this);
    return true;
}

void DeferredRenderPass::Render(RenderGraphBuilder& graph_builder,
                                RenderGraphTextureHandle color,
                                RenderGraphTextureHandle depth,
                                const ShadowPass::Result& shadows,
                                const std::array<int, 2>& viewport_size)
{
    if (m_gbuffer_tex_shader_program == nullptr || m_gbuffer_no_tex_shader_program == nullptr || m_lighting_shader_program == nullptr)
        return;

    const GLsizei width = viewport_size[0];
    const GLsizei height = viewport_size[1];

    // G-buffer attachment order: position, normal, albedo, MRA, emissive.
    const auto gbuffer_desc = [width, height](GLint internal_format, GLenum format, GLenum type)
    {
        return TextureDescription{.width = width, .height = height, .internal_format = internal_format, .format = format, .type = type};
    };

    auto* geometry = graph_builder.AllocateParameter<DeferredGeometryParameter>();
    geometry->position = graph_builder.CreateTexture("GBuffer.Position", gbuffer_desc(GL_RGB16F, GL_RGB, GL_FLOAT));
    geometry->normal = graph_builder.CreateTexture("GBuffer.Normal", gbuffer_desc(GL_RGB16F, GL_RGB, GL_FLOAT));
    geometry->albedo = graph_builder.CreateTexture("GBuffer.Albedo", gbuffer_desc(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE));
    geometry->mra = graph_builder.CreateTexture("GBuffer.MRA", gbuffer_desc(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE));
    geometry->emissive = graph_builder.CreateTexture("GBuffer.Emissive", gbuffer_desc(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE));
    geometry->depth = depth;

    auto* lighting = graph_builder.AllocateParameter<DeferredLightingParameter>();
    lighting->position = geometry->position;
    lighting->normal = geometry->normal;
    lighting->albedo = geometry->albedo;
    lighting->mra = geometry->mra;
    lighting->emissive = geometry->emissive;
    lighting->color = color;
    lighting->shadows = shadows;

    graph_builder.AddPass(
        "DeferredGeometry",
        [geometry](RenderGraphPass& pass)
        {
            pass.Produce(geometry->position);
            pass.Produce(geometry->normal);
            pass.Produce(geometry->albedo);
            pass.Produce(geometry->mra);
            pass.Produce(geometry->emissive);
            pass.Produce(geometry->depth);
        },
        [geometry, this](RenderGraphContext& context)
        {
            FrameBufferObject& target = context.GetFrameBuffer(
                {geometry->position, geometry->normal, geometry->albedo, geometry->mra, geometry->emissive},
                geometry->depth);
            RenderGeometryPass(context.GetContextState(), target);
        });

    graph_builder.AddPass(
        "DeferredLighting",
        [lighting](RenderGraphPass& pass)
        {
            pass.Consume(lighting->position);
            pass.Consume(lighting->normal);
            pass.Consume(lighting->albedo);
            pass.Consume(lighting->mra);
            pass.Consume(lighting->emissive);
            pass.Consume(lighting->shadows.csm);
            pass.Consume(lighting->shadows.point_shadows);
            pass.Produce(lighting->color);
        },
        [lighting, this](RenderGraphContext& context)
        {
            LazyUpdateLightData();
            // Ensure the deferred UBO is bound when both forward/deferred exist.
            m_light_ubo->BindBase(0);

            auto scene = SceneManager::GetInstance().GetScene();
            if (!scene) return;
            const auto& settings = scene->GetRenderSettings();

            FrameBufferObject& target = context.GetFrameBuffer({lighting->color});
            const std::array<Texture*, 5> gbuffer = {&context.GetTexture(lighting->position),
                                                    &context.GetTexture(lighting->normal),
                                                    &context.GetTexture(lighting->albedo),
                                                    &context.GetTexture(lighting->mra),
                                                    &context.GetTexture(lighting->emissive)};
            RenderLightingPass(context.GetContextState(), settings, lighting->shadows,
                               context.GetTexture(lighting->shadows.point_shadows),
                               context.GetTexture(lighting->shadows.csm),
                               target, gbuffer);
        });
}

void DeferredRenderPass::OnLightChanged()
{
    m_lighting_data_dirty = true;
}

void DeferredRenderPass::LazyUpdateLightData()
{
    if (m_lighting_data_dirty)
    {
        const auto& point_lights = LightManager::GetInstance().GetLights(Light::Type::Point);
        LightUBOData light_ubo_data = {};
        for (const auto& point_light_ptr : point_lights)
        {
            auto light = point_light_ptr.second.lock();
            if (!light) continue;
            if (light_ubo_data.num_lights >= LightUBOData::kMaxLights)
                break;

            light_ubo_data.lights[light_ubo_data.num_lights].light_pos = light->GetPosition();
            light_ubo_data.lights[light_ubo_data.num_lights].light_color = light->GetColor();
            light_ubo_data.lights[light_ubo_data.num_lights].light_intensity = light->GetIntensity();
            light_ubo_data.lights[light_ubo_data.num_lights].cull_distance = light->GetCullDistance();
            ++light_ubo_data.num_lights;
        }
        m_light_ubo->SetData(&light_ubo_data, sizeof(LightUBOData));

        m_lighting_data_dirty = false;
    }
}

void DeferredRenderPass::RenderGeometryPass(ContextState& context_state, FrameBufferObject& target) const
{
    target.Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderState render_state;
    render_state.depth_stencil_state.depth_test_enabled = true;
    render_state.depth_stencil_state.depth_write_enabled = true;
    context_state.ApplyRenderState(render_state);

    std::vector<std::shared_ptr<Mesh>> meshes = SceneManager::GetInstance().GetMeshesInViewFrustum();

    auto mesh_split = std::partition(meshes.begin(), meshes.end(),
        [](const std::shared_ptr<Mesh>& mesh) {
            return mesh->HasTextures();
        });

    auto mesh_shader_program = m_gbuffer_tex_shader_program.get();
    bool program_uninitialized = true;
    auto& texture_manager = TextureManager::GetInstance();
    auto& dummy_white = texture_manager.GetDummyWhiteTexture();
    auto& dummy_black = texture_manager.GetDummyBlackTexture();
    auto& dummy_normal = texture_manager.GetDummyNormalTexture();
    for (const auto& mesh : meshes)
    {
        if (mesh_split != meshes.end() && mesh == *mesh_split)
        {
            mesh_shader_program = m_gbuffer_no_tex_shader_program.get();
            program_uninitialized = true;
        }

        if (program_uninitialized)
        {
            mesh_shader_program->Bind();
            mesh_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
            mesh_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
            program_uninitialized = false;
        }

        auto p_material = mesh->m_render_proxy.lock();
        auto material = std::dynamic_pointer_cast<MeshRenderMaterial>(p_material);
        if (!material) continue;

        const auto material_owner = material->GetOwner().lock();
        SCOPED_RENDER_EVENT(material_owner ? material_owner->GetName() : "mesh render material");

        const glm::mat4 model = material->GetModelMatrix();
        mesh_shader_program->SetUniform("uModel", model);

        int current_unit = 1;
        int material_texture_unit_begin = current_unit;
        for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
        {
            current_unit = material_texture_unit_begin;
            material->m_vaos[i]->Bind();

            if (material->m_mesh->HasTextures())
            {
                const auto& submesh = material->m_mesh->m_submeshes[i];
                // Match Forward's PBR/Phong decision so both paths render the same material model.
                const bool use_pbr = material->m_mesh->HasPBRTextures();
                mesh_shader_program->SetUniform("uUsePBR", use_pbr);

                if (use_pbr)
                {
                    BindPbrTextures(submesh, texture_manager, *mesh_shader_program, current_unit);
                }
                else
                {
                    bool has_normal_map = false;
                    bool has_diffuse_map = false;
                    bool has_specular_map = false;
                    for (int j = 0; j < submesh.m_textures.size(); ++j)
                    {
                        auto& surface_texture = texture_manager.GetTexture(submesh.m_textures[j]);
                        surface_texture.texture.Bind(current_unit++);
                        mesh_shader_program->SetUniform(std::string("uTex") + surface_texture.type, current_unit - 1);
                        if (surface_texture.type == "Normal")
                        {
                            has_normal_map = true;
                        }
                        else if (surface_texture.type == "Diffuse")
                        {
                            has_diffuse_map = true;
                        }
                        else if (surface_texture.type == "Specular")
                        {
                            has_specular_map = true;
                        }
                    }

                    if (!has_diffuse_map)
                    {
                        dummy_white.texture.Bind(current_unit++);
                        mesh_shader_program->SetUniform("uTexDiffuse", current_unit - 1);
                    }
                    if (!has_specular_map)
                    {
                        dummy_black.texture.Bind(current_unit++);
                        mesh_shader_program->SetUniform("uTexSpecular", current_unit - 1);
                    }

                    mesh_shader_program->SetUniform("uHasNormalMap", has_normal_map);
                    if (!has_normal_map)
                    {
                        dummy_normal.texture.Bind(current_unit++);
                        mesh_shader_program->SetUniform("uTexNormal", current_unit - 1);
                    }

                    mesh_shader_program->SetUniform("uHasBaseColorMap", false);
                    mesh_shader_program->SetUniform("uHasMetalnessMap", false);
                    mesh_shader_program->SetUniform("uHasRoughnessMap", false);
                    mesh_shader_program->SetUniform("uHasAoMap", false);
                    mesh_shader_program->SetUniform("uHasEmissiveMap", false);
                }
            }
            else
            {
                mesh_shader_program->SetUniform("uUsePBR", false);
                mesh_shader_program->SetUniform("uColor", material->m_mesh->m_submeshes[i].m_color);
            }

            glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
            material->m_vaos[i]->Unbind();
        }
    }
}

void DeferredRenderPass::RenderLightingPass(ContextState& context_state,
                                           const RenderSettings& settings,
                                           const ShadowPass::Result& shadows,
                                           Texture& point_shadows,
                                           Texture& directional_shadows,
                                           FrameBufferObject& target,
                                           const std::array<Texture*, 5>& gbuffer) const
{
    target.Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    RenderState render_state;
    render_state.depth_stencil_state.depth_test_enabled = false;
    render_state.depth_stencil_state.depth_write_enabled = false;
    render_state.raster_state.cull_enabled = false;
    context_state.ApplyRenderState(render_state);

    m_lighting_shader_program->Bind();

    // G-buffer bindings
    for (int i = 0; i < static_cast<int>(gbuffer.size()); ++i)
    {
        gbuffer[i]->Bind(i);
    }
    m_lighting_shader_program->SetUniform("uGBufferPosition", 0);
    m_lighting_shader_program->SetUniform("uGBufferNormal", 1);
    m_lighting_shader_program->SetUniform("uGBufferAlbedo", 2);
    m_lighting_shader_program->SetUniform("uGBufferMRA", 3);
    m_lighting_shader_program->SetUniform("uGBufferEmissive", 4);

    int current_unit = 5;
    m_lighting_shader_program->SetUniform("uTexPointLightShadowMaps", current_unit);
    point_shadows.Bind(current_unit++);
    m_lighting_shader_program->SetUniform("uDirectionalLightTexShadowMap", current_unit);
    directional_shadows.Bind(current_unit++);

    m_lighting_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
    m_lighting_shader_program->SetUniform("uViewPos", MainCamera::GetInstance().GetPosition());
    m_lighting_shader_program->SetUniform("uDebugShowCascades", GlobalContext::GetInstance().draw_directional_cascades);
    m_lighting_shader_program->SetUniform("uDirectionalShadowPcfSamples", settings.GetDirectionalShadowPcfSamples());
    m_lighting_shader_program->SetUniform("uPointShadowPcfSamples", settings.GetPointShadowPcfSamples());

    if (shadows.directional_light && shadows.cascades_valid)
    {
        const int cascade_count = settings.GetDirectionalCascadeCount();
        for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index)
        {
            const auto cascade_str = std::to_string(cascade_index);
            m_lighting_shader_program->SetUniform("uDirectionalLightOrthoVP[" + cascade_str + "]", shadows.cascades[cascade_index].vp);
            m_lighting_shader_program->SetUniform("uDirectionalLightCascadeSplits[" + cascade_str + "]", shadows.cascades[cascade_index].split_depth);
            m_lighting_shader_program->SetUniform("uDirectionalLightNearFarNorm[" + cascade_str + "]", shadows.cascades[cascade_index].near_far_norm);
        }
        m_lighting_shader_program->SetUniform("uDirectionalCascadeCount", cascade_count);
        m_lighting_shader_program->SetUniform("uDirectionalLightDirection", shadows.directional_light->GetDirection());
        m_lighting_shader_program->SetUniform("uDirectionalLightColor", shadows.directional_light->GetColor());
        m_lighting_shader_program->SetUniform("uDirectionalLightIntensity", shadows.directional_light->GetIntensity());
        m_lighting_shader_program->SetUniform("uEnableDirectionalLightShadow", true);
    }
    else
    {
        m_lighting_shader_program->SetUniform("uEnableDirectionalLightShadow", false);
    }

    DrawQuad();
    m_lighting_shader_program->Unbind();
}

} // namespace Aurora
