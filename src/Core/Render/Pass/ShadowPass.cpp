// std include
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/ShadowPass.h"
#include "Core/Render/LightUBO.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "Core/Render/RenderGraph/RenderGraphBuilder.h"
#include "Core/Render/RenderGraph/RenderGraphContext.h"
#include "Runtime/Scene/Components/Mesh.h"
#include "Utility/FileSystem.h"
#include "glWrapper/FrameBufferObject.h"
#include "glWrapper/RenderEventInfo.h"
#include "glWrapper/Shader.h"
#include "Runtime/Scene/Camera.h"
#include "Runtime/Scene/LightManager.h"
#include "Runtime/Scene/RenderSettings.h"
#include "Runtime/Scene/SceneManager.h"

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

struct ShadowParameter : PassParameterBase
{
    RenderGraphTextureHandle csm;
    RenderGraphTextureHandle point_shadows;
    std::array<int, 2> resolution = {0, 0};
    std::shared_ptr<Light> directional_light;
    bool cascades_valid = false;
    std::array<ShadowPass::DirectionalCascadeData, ShadowPass::kDirectionalCascadeMax> cascades;
};

// Only one directional light casts a shadow, so the shading pass must use the same light the
// cascades were fitted to.
std::shared_ptr<Light> FindBrightestDirectionalLight()
{
    std::shared_ptr<Light> brightest;
    float max_intensity = 0.0f;
    for (const auto& [id, weak_light] : LightManager::GetInstance().GetLights(Light::Type::Directional))
    {
        auto light = weak_light.lock();
        if (light == nullptr) continue;
        if (light->GetIntensity() > max_intensity)
        {
            max_intensity = light->GetIntensity();
            brightest = light;
        }
    }
    return brightest;
}

// The point-light shadow cubemap array needs one cube per light the shader will sample.
uint32_t CountShadowCastingPointLights()
{
    uint32_t count = 0;
    for (const auto& [id, weak_light] : LightManager::GetInstance().GetLights(Light::Type::Point))
    {
        if (weak_light.expired()) continue;
        if (++count >= LightUBOData::kMaxLights) break;
    }
    return count;
}
} // namespace

bool ShadowPass::Init()
{
    std::vector<Shader> shaders;
    shaders.emplace_back(ShaderType::VertexShader);
    const auto vert_path = FileSystem::GetFullPath("shaders/mesh.vert");
    if (!shaders[0].Load(vert_path))
    {
        spdlog::error("Failed to load vertex shader {}", vert_path);
        return false;
    }
    shaders.emplace_back(ShaderType::FragmentShader);
    const auto frag_path = FileSystem::GetFullPath("shaders/pure.frag");
    if (!shaders[1].Load(frag_path))
    {
        spdlog::error("Failed to load fragment shader {}", frag_path);
        return false;
    }
    m_shadow_map_shader_program = std::make_unique<ShaderProgram>();
    if (!m_shadow_map_shader_program->Load(shaders))
    {
        spdlog::error("Failed to load shadow map shader program");
        return false;
    }

    return true;
}

void ShadowPass::UpdateDirectionalLightCascades(Result& result, const RenderSettings& settings) const
{
    result.cascades_valid = false;
    if (result.directional_light == nullptr)
        return;

    const auto& camera = MainCamera::GetInstance();
    const float near_clip = camera.GetNearPlane();
    const float far_clip = camera.GetFarPlane();
    if (near_clip <= 0.0f || far_clip <= near_clip)
        return;
    const int cascade_count = settings.GetDirectionalCascadeCount();
    if (cascade_count <= 0)
        return;

    const glm::mat4 view = camera.GetViewMatrix();
    const glm::mat4 inv_view = glm::inverse(view);
    const glm::mat4 proj = camera.GetProjectionMatrix();
    const float tan_half_fov = 1.0f / proj[1][1];
    const float aspect = proj[1][1] / proj[0][0];

    const float clip_range = far_clip - near_clip;
    const float ratio = far_clip / near_clip;

    std::array<float, kDirectionalCascadeMax> splits = {};
    const float split_lambda = settings.GetDirectionalSplitLambda();
    for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index)
    {
        const float p = (cascade_index + 1) / static_cast<float>(cascade_count);
        const float log_split = near_clip * std::pow(ratio, p);
        const float linear_split = near_clip + clip_range * p;
        splits[cascade_index] = split_lambda * log_split + (1.0f - split_lambda) * linear_split;
    }

    const glm::vec3 light_dir = glm::normalize(result.directional_light->GetDirection());
    const glm::vec3 up_dir = (std::abs(glm::dot(light_dir, glm::vec3(0.f, 1.f, 0.f))) > 0.999f)
                                 ? glm::vec3(0.f, 0.f, 1.f)
                                 : glm::vec3(0.f, 1.f, 0.f);

    float prev_split = near_clip;
    const float padding_xy = settings.GetDirectionalPaddingXY();
    const float padding_z = settings.GetDirectionalPaddingZ();
    const int shadow_map_size = std::max(settings.GetDirectionalShadowMapSize(), 1);
    for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index)
    {
        const float split_dist = splits[cascade_index];
        const float near_plane = prev_split;
        const float far_plane = split_dist;

        const float near_height = 2.0f * tan_half_fov * near_plane;
        const float near_width = near_height * aspect;
        const float far_height = 2.0f * tan_half_fov * far_plane;
        const float far_width = far_height * aspect;

        std::array<glm::vec4, 8> corners = {
            glm::vec4(-near_width / 2.0f, -near_height / 2.0f, -near_plane, 1.0f),
            glm::vec4(near_width / 2.0f, -near_height / 2.0f, -near_plane, 1.0f),
            glm::vec4(near_width / 2.0f, near_height / 2.0f, -near_plane, 1.0f),
            glm::vec4(-near_width / 2.0f, near_height / 2.0f, -near_plane, 1.0f),
            glm::vec4(-far_width / 2.0f, -far_height / 2.0f, -far_plane, 1.0f),
            glm::vec4(far_width / 2.0f, -far_height / 2.0f, -far_plane, 1.0f),
            glm::vec4(far_width / 2.0f, far_height / 2.0f, -far_plane, 1.0f),
            glm::vec4(-far_width / 2.0f, far_height / 2.0f, -far_plane, 1.0f)};

        glm::vec3 center(0.0f);
        for (auto& corner : corners)
        {
            corner = inv_view * corner;
            center += glm::vec3(corner);
        }
        center /= 8.0f;

        float radius = 0.0f;
        for (const auto& corner : corners)
        {
            radius = std::max(radius, glm::length(glm::vec3(corner) - center));
        }

        // Build a rotation-only light view to snap the cascade center in light space.
        const glm::mat4 light_view_rot = glm::lookAt(glm::vec3(0.0f), light_dir, up_dir);
        const float ortho_extent = radius + padding_xy;
        const float texel_size = (2.0f * ortho_extent) / static_cast<float>(shadow_map_size);
        glm::vec4 center_ls = light_view_rot * glm::vec4(center, 1.0f);
        center_ls.x = std::round(center_ls.x / texel_size) * texel_size;
        center_ls.y = std::round(center_ls.y / texel_size) * texel_size;
        const glm::vec3 snapped_center = glm::vec3(glm::inverse(light_view_rot) * center_ls);

        const glm::mat4 light_view = glm::lookAt(snapped_center - light_dir, snapped_center, up_dir);

        float min_z = std::numeric_limits<float>::max();
        float max_z = std::numeric_limits<float>::lowest();

        for (const auto& corner : corners)
        {
            const glm::vec4 corner_ls = light_view * corner;
            min_z = std::min(min_z, corner_ls.z);
            max_z = std::max(max_z, corner_ls.z);
        }

        // Fix XY extent to a stable size so the projection does not breathe as the camera
        // moves; pad Z to stabilize the depth range.
        const float min_x = -ortho_extent;
        const float max_x = ortho_extent;
        const float min_y = -ortho_extent;
        const float max_y = ortho_extent;
        min_z -= padding_z;
        max_z += padding_z;

        const glm::mat4 light_proj = glm::ortho(min_x, max_x, min_y, max_y, min_z, max_z);
        const AxisAlignedBoundingBox light_aabb(glm::vec3(min_x, min_y, min_z), glm::vec3(max_x, max_y, max_z));
        const glm::mat4 inv_light_view = glm::inverse(light_view);

        result.cascades[cascade_index].view = light_view;
        result.cascades[cascade_index].proj = light_proj;
        result.cascades[cascade_index].vp = light_proj * light_view;
        result.cascades[cascade_index].split_depth = split_dist;
        result.cascades[cascade_index].near_far_norm = max_z - min_z;
        result.cascades[cascade_index].world_aabb = inv_light_view * light_aabb;
        prev_split = split_dist;
    }

    result.cascades_valid = true;
}

ShadowPass::Result ShadowPass::Render(RenderGraphBuilder& graph_builder)
{
    Result result;
    auto scene = SceneManager::GetInstance().GetScene();
    if (!scene) return result;

    const auto& settings = scene->GetRenderSettings();
    const int shadow_map_size = std::max(settings.GetDirectionalShadowMapSize(), 1);
    const uint32_t point_light_count = std::max(CountShadowCastingPointLights(), 1u);

    // Setup phase: sizes and cascade fitting happen here, not during execution.
    result.directional_light = FindBrightestDirectionalLight();
    UpdateDirectionalLightCascades(result, settings);
    result.resolution = {shadow_map_size, shadow_map_size};

    result.csm = graph_builder.CreateTexture(
        "Shadow.CSM",
        {.width = shadow_map_size,
         .height = shadow_map_size,
         .texture_type = TextureType::Texture2DArray,
         .layers = std::max(settings.GetDirectionalCascadeCount(), 1),
         .internal_format = GL_DEPTH_COMPONENT,
         .format = GL_DEPTH_COMPONENT,
         .type = GL_FLOAT,
         .wrap_s = TextureWrapMode::ClampToEdge,
         .wrap_t = TextureWrapMode::ClampToEdge,
         .wrap_r = TextureWrapMode::ClampToEdge});

    result.point_shadows = graph_builder.CreateTexture(
        "Shadow.Point",
        {.width = 1024,
         .height = 1024,
         .texture_type = TextureType::CubemapArray,
         .layers = static_cast<GLsizei>(point_light_count),
         .internal_format = GL_DEPTH_COMPONENT,
         .format = GL_DEPTH_COMPONENT,
         .type = GL_FLOAT,
         .wrap_s = TextureWrapMode::ClampToEdge,
         .wrap_t = TextureWrapMode::ClampToEdge,
         .wrap_r = TextureWrapMode::ClampToEdge});

    auto* param = graph_builder.AllocateParameter<ShadowParameter>();
    param->csm = result.csm;
    param->point_shadows = result.point_shadows;
    param->resolution = result.resolution;
    param->directional_light = result.directional_light;
    param->cascades_valid = result.cascades_valid;
    param->cascades = result.cascades;

    ShaderProgram* shader = m_shadow_map_shader_program.get();
    if (shader == nullptr) return result;

    graph_builder.AddPass(
        "DirectionalShadow",
        [param](RenderGraphPass& pass)
        {
            pass.Produce(param->csm);
        },
        [param, shader](RenderGraphContext& context)
        {
            if (!param->cascades_valid) return;

            SCOPED_RENDER_EVENT("Directional Light Shadow Pass");

            RenderState render_state;
            render_state.depth_stencil_state.depth_test_enabled = true;
            render_state.depth_stencil_state.depth_write_enabled = true;
            context.ApplyRenderState(render_state);

            FrameBufferObject& fbo = context.GetFrameBuffer({}, param->csm, param->resolution);
            shader->Bind();

            const auto& meshes = SceneManager::GetInstance().GetMeshes();
            struct ShadowRenderItem
            {
                std::shared_ptr<MeshRenderMaterial> material;
                AxisAlignedBoundingBox aabb;
            };

            std::vector<ShadowRenderItem> shadow_items;
            shadow_items.reserve(meshes.size());
            for (const auto& mesh_it : meshes)
            {
                const auto p_mesh = mesh_it.second.lock();
                if (!p_mesh) continue;
                const auto material = std::dynamic_pointer_cast<MeshRenderMaterial>(p_mesh->m_render_proxy.lock());
                if (!material) continue;
                shadow_items.push_back({material, p_mesh->GetAABB()});
            }

            auto scene = SceneManager::GetInstance().GetScene();
            const int cascade_count = scene ? scene->GetRenderSettings().GetDirectionalCascadeCount() : 0;
            for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index)
            {
                const std::string cascade_label = "Directional Cascade " + std::to_string(cascade_index);
                SCOPED_RENDER_EVENT(cascade_label);

                fbo.BindDepthTextureLayer(static_cast<unsigned int>(cascade_index));
                glClear(GL_DEPTH_BUFFER_BIT);
                shader->SetUniform("uProjection", param->cascades[cascade_index].proj);
                shader->SetUniform("uView", param->cascades[cascade_index].view);

                const auto& cascade_aabb = param->cascades[cascade_index].world_aabb;
                const bool cull_enabled = cascade_aabb.IsValid();
                for (const auto& item : shadow_items)
                {
                    if (cull_enabled && item.aabb.IsValid() && !cascade_aabb.Intersects(item.aabb))
                        continue;

                    const auto& material = item.material;
                    const auto material_owner = material->GetOwner().lock();
                    SCOPED_RENDER_EVENT(material_owner ? material_owner->GetName() : "mesh render material");

                    shader->SetUniform("uModel", material->GetModelMatrix());
                    for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
                    {
                        material->m_vaos[i]->Bind();
                        glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                        material->m_vaos[i]->Unbind();
                    }
                }
            }

            shader->Unbind();
        });

    graph_builder.AddPass(
        "PointLightShadow",
        [param](RenderGraphPass& pass)
        {
            pass.Produce(param->point_shadows);
        },
        [param, shader](RenderGraphContext& context)
        {
            const auto& lights = LightManager::GetInstance().GetLights(Light::Type::Point);
            if (lights.empty()) return;

            SCOPED_RENDER_EVENT("Point Light Shadow Pass");

            FrameBufferObject& fbo = context.GetFrameBuffer({}, param->point_shadows, {1024, 1024});
            int light_index = 0;
            for (const auto& point_light : lights)
            {
                auto light = point_light.second.lock();
                if (light == nullptr) continue;

                const auto light_owner = light->GetOwner().lock();
                SCOPED_RENDER_EVENT(light_owner ? light_owner->GetName() : "light");

                RenderState render_state;
                render_state.depth_stencil_state.depth_test_enabled = true;
                render_state.depth_stencil_state.depth_write_enabled = true;
                context.ApplyRenderState(render_state);

                for (int face = 0; face < static_cast<int>(k_dirs.size()); ++face)
                {
                    fbo.BindDepthCubemapArray(light_index, face);
                    glClear(GL_DEPTH_BUFFER_BIT);
                }

                const float near_plane = light->GetCullDistance().x;
                const float far_plane = light->GetCullDistance().y;
                const auto perspective = glm::perspective(glm::radians(90.f), 1.f, near_plane, far_plane);

                shader->Bind();
                shader->SetUniform("uProjection", perspective);

                for (const auto& mesh_it : SceneManager::GetInstance().GetMeshes())
                {
                    const auto p_mesh = mesh_it.second.lock();
                    if (!p_mesh) continue;
                    const auto material = std::dynamic_pointer_cast<MeshRenderMaterial>(p_mesh->m_render_proxy.lock());
                    if (!material) continue;

                    const auto material_owner = material->GetOwner().lock();
                    SCOPED_RENDER_EVENT(material_owner ? material_owner->GetName() : "mesh render material");

                    shader->SetUniform("uModel", material->GetModelMatrix());
                    for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
                    {
                        material->m_vaos[i]->Bind();
                        for (size_t face = 0; face < k_dirs.size(); ++face)
                        {
                            shader->SetUniform("uView", glm::lookAt(light->GetPosition(), light->GetPosition() + k_dirs[face], k_ups[face]));
                            fbo.BindDepthCubemapArray(light_index, static_cast<unsigned int>(face));
                            glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                        }
                        material->m_vaos[i]->Unbind();
                    }
                }
                shader->Unbind();
                ++light_index;
            }
        });

    return result;
}

} // namespace Aurora
