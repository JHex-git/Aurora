// std include
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/ForwardRenderPass.h"
#include "Core/Render/LightUBO.h"
#include "Core/Render/PbrUtils.h"
#include "Core/Render/RenderGraph/RenderGraphBuilder.h"
#include "Core/Render/RenderGraph/RenderGraphContext.h"
#include "Utility/FileSystem.h"
#include "glWrapper/Shader.h"
#include "glWrapper/FrameBufferObject.h"
#include "glWrapper/RenderEventInfo.h"
#include "Runtime/Scene/Camera.h"
#include "Runtime/Scene/LightManager.h"
#include "Runtime/Scene/TextureManager.h"
#include "Runtime/GlobalContext.h"
#include "glWrapper/Utils.h"
#include "glWrapper/UniformBuffer.h"
#include "Runtime/Scene/SceneManager.h"
#include "Runtime/Scene/RenderSettings.h"

namespace Aurora
{

namespace
{
struct ForwardParameter : PassParameterBase
{
    RenderGraphTextureHandle color;
    RenderGraphTextureHandle depth;
    ShadowPass::Result shadows;
};
} // namespace

ForwardRenderPass::ForwardRenderPass() = default;

bool ForwardRenderPass::Init()
{
    m_light_ubo = std::make_unique<UniformBuffer>(sizeof(LightUBOData), 0, GL_STATIC_DRAW);

    // shader program with texture for point light
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
        shaders[1].SetOption("DIRECTIONAL_CASCADE_COUNT", kDirectionalCascadeMax);
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_tex_mesh_shader_program = std::make_unique<ShaderProgram>();
        if (!m_tex_mesh_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    // shader program without texture for point light
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
        shaders[1].SetOption("DIRECTIONAL_CASCADE_COUNT", kDirectionalCascadeMax);
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_no_tex_mesh_shader_program = std::make_unique<ShaderProgram>();
        if (!m_no_tex_mesh_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    LightManager::GetInstance().AddLightEventListener(this);
    return true;
}

void ForwardRenderPass::Render(RenderGraphBuilder& graph_builder,
                               RenderGraphTextureHandle color,
                               RenderGraphTextureHandle depth,
                               const ShadowPass::Result& shadows,
                               const std::array<int, 2>& viewport_size)
{
    if (m_tex_mesh_shader_program == nullptr || m_no_tex_mesh_shader_program == nullptr) return;

    auto* param = graph_builder.AllocateParameter<ForwardParameter>();
    param->color = color;
    param->depth = depth;
    param->shadows = shadows;

    graph_builder.AddPass(
        "Forward",
        [param](RenderGraphPass& pass)
        {
            pass.Produce(param->color);
            pass.Produce(param->depth);
            pass.Consume(param->shadows.csm);
            pass.Consume(param->shadows.point_shadows);
        },
        [param, this](RenderGraphContext& context)
        {
            LazyUpdateLightData();
            // Ensure the forward UBO is bound when both forward/deferred exist.
            m_light_ubo->BindBase(0);

            auto scene = SceneManager::GetInstance().GetScene();
            if (!scene) return;
            const auto& settings = scene->GetRenderSettings();

            FrameBufferObject& target = context.GetFrameBuffer({param->color}, param->depth);
            RenderForwardShading(context.GetContextState(), settings, param->shadows,
                                 context.GetTexture(param->shadows.point_shadows),
                                 context.GetTexture(param->shadows.csm),
                                 target);
        });
}

void ForwardRenderPass::OnLightChanged()
{
    m_lighting_data_dirty = true;
}

void ForwardRenderPass::LazyUpdateLightData()
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

            // fill light data to light_ubo_data
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

void ForwardRenderPass::RenderForwardShading(ContextState& context_state, const RenderSettings& settings, const ShadowPass::Result& shadows, Texture& point_shadows, Texture& directional_shadows, FrameBufferObject& target) const
{
    target.Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Only meshes with BVH intersected with view frustum should be rendered
    std::vector<std::shared_ptr<Mesh>> meshes = SceneManager::GetInstance().GetMeshesInViewFrustum();

    // Split meshes into three groups: PBR textures, non-PBR textures, and no textures
    // This is to minimize shader program switches
    auto pbr_split = std::partition(meshes.begin(), meshes.end(),
        [](const std::shared_ptr<Mesh>& mesh) {
            return mesh->HasPBRTextures();
        });

    auto textured_split = std::partition(pbr_split, meshes.end(),
        [](const std::shared_ptr<Mesh>& mesh) {
            return mesh->HasTextures();
        });

    auto init_program = [&](ShaderProgram* mesh_shader_program)
    {
        mesh_shader_program->Bind();
        mesh_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
        mesh_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
        mesh_shader_program->SetUniform("uViewPos", MainCamera::GetInstance().GetPosition());
        mesh_shader_program->SetUniform("uDebugShowCascades", GlobalContext::GetInstance().draw_directional_cascades);
        if (shadows.directional_light && shadows.cascades_valid)
        {
            const int cascade_count = settings.GetDirectionalCascadeCount();
            for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index)
            {
                const auto cascade_str = std::to_string(cascade_index);
                mesh_shader_program->SetUniform("uDirectionalLightOrthoVP[" + cascade_str + "]", shadows.cascades[cascade_index].vp);
                mesh_shader_program->SetUniform("uDirectionalLightCascadeSplits[" + cascade_str + "]", shadows.cascades[cascade_index].split_depth);
                mesh_shader_program->SetUniform("uDirectionalLightNearFarNorm[" + cascade_str + "]", shadows.cascades[cascade_index].near_far_norm);
            }
            mesh_shader_program->SetUniform("uDirectionalCascadeCount", cascade_count);
            mesh_shader_program->SetUniform("uDirectionalLightDirection", shadows.directional_light->GetDirection());
            mesh_shader_program->SetUniform("uDirectionalLightColor", shadows.directional_light->GetColor());
            mesh_shader_program->SetUniform("uDirectionalLightIntensity", shadows.directional_light->GetIntensity());
            mesh_shader_program->SetUniform("uEnableDirectionalLightShadow", true);
        }
        else
        {
            mesh_shader_program->SetUniform("uEnableDirectionalLightShadow", false);
        }
        mesh_shader_program->SetUniform("uDirectionalShadowPcfSamples", settings.GetDirectionalShadowPcfSamples());
        mesh_shader_program->SetUniform("uPointShadowPcfSamples", settings.GetPointShadowPcfSamples());
    };

    // Forward shading uses one fragment shader for both Phong and PBR; uUsePBR selects the path.
    auto render_meshes = [&](auto begin, auto end, ShaderProgram* mesh_shader_program, bool use_pbr, bool use_textures)
    {
        if (begin == end || mesh_shader_program == nullptr) return;

        init_program(mesh_shader_program);
        for (auto it = begin; it != end; ++it)
        {
            const auto& mesh = *it;
            auto p_material = mesh->m_render_proxy.lock();
            auto material = std::dynamic_pointer_cast<MeshRenderMaterial>(p_material);
            if (!material) continue;

            const auto material_owner = material->GetOwner().lock();
            SCOPED_RENDER_EVENT(material_owner ? material_owner->GetName() : "mesh render material");

            const glm::mat4 model = material->GetModelMatrix();
            mesh_shader_program->SetUniform("uModel", model);

            int current_unit = 1;
            // use shadow map to decide the light contribution
            point_shadows.Bind(current_unit++);
            mesh_shader_program->SetUniform("uTexPointLightShadowMaps", current_unit - 1);
            directional_shadows.Bind(current_unit++);
            mesh_shader_program->SetUniform("uDirectionalLightTexShadowMap", current_unit - 1);
            int material_texture_unit_begin = current_unit;

            for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
            {
                current_unit = material_texture_unit_begin;
                material->m_vaos[i]->Bind();

                if (use_pbr)
                {
                    // PBR path
                    mesh_shader_program->SetUniform("uUsePBR", true);
                    const auto& submesh = material->m_mesh->m_submeshes[i];
                    auto& texture_manager = TextureManager::GetInstance();
                    BindPbrTextures(submesh, texture_manager, *mesh_shader_program, current_unit);
                }
                else if (use_textures)
                {
                    // Phong path with textures.
                    mesh_shader_program->SetUniform("uUsePBR", false);
                    bool has_normal_map = false;
                    for (int j = 0; j < material->m_mesh->m_submeshes[i].m_textures.size(); ++j)
                    {
                        auto& surface_texture = TextureManager::GetInstance().GetTexture(material->m_mesh->m_submeshes[i].m_textures[j]);
                        // reserve unit 0 for temporary use
                        surface_texture.texture.Bind(current_unit++);
                        mesh_shader_program->SetUniform(std::string("uTex") + surface_texture.type, current_unit - 1);
                        if (surface_texture.type == "Normal")
                        {
                            has_normal_map = true;
                        }
                    }
                    if (!has_normal_map)
                    {
                        // Use a flat normal map when the material doesn't provide one.
                        auto& normal_texture = TextureManager::GetInstance().GetDummyNormalTexture();
                        normal_texture.texture.Bind(current_unit++);
                        mesh_shader_program->SetUniform("uTexNormal", current_unit - 1);
                    }
                }
                else
                {
                    // Phong path without textures (solid color).
                    mesh_shader_program->SetUniform("uColor", material->m_mesh->m_submeshes[i].m_color);
                }

                glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                material->m_vaos[i]->Unbind();
            }
        }
    };

    render_meshes(meshes.begin(), pbr_split, m_tex_mesh_shader_program.get(), true, true);
    render_meshes(pbr_split, textured_split, m_tex_mesh_shader_program.get(), false, true);
    render_meshes(textured_split, meshes.end(), m_no_tex_mesh_shader_program.get(), false, false);
}

} // namespace Aurora
