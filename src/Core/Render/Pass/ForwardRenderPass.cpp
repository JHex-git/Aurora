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
#include "Utility/FileSystem.h"
#include "glWrapper/Shader.h"
#include "glWrapper/RenderEventInfo.h"
#include "Runtime/Scene/Camera.h"
#include "Runtime/Scene/LightManager.h"
#include "Runtime/Scene/TextureManager.h"
#include "glWrapper/Utils.h"
#include "glWrapper/UniformBuffer.h"
#include "Runtime/Scene/SceneManager.h"
#include "Runtime/Scene/RenderSettings.h"

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

ForwardRenderPass::ForwardRenderPass() = default;

bool ForwardRenderPass::Init(const std::array<int, 2>& viewport_size)
{
    if (!RenderPass::Init(viewport_size)) return false;
    
    auto fbo = FrameBufferObjectBuilder(viewport_size[0], viewport_size[1])
                                        .AddColorAttachment({.internal_format = GL_RGBA16F, .format = GL_RGBA, .type = GL_FLOAT})
                                        .EnableDepthAttachment({}).Create();
    if (!fbo.has_value()) return false;
    m_fbo = std::make_shared<FrameBufferObject>(std::move(fbo.value()));

    auto shadow_cubemap_fbo = FrameBufferObjectBuilder(1024, 1024).EnableDepthAttachment({.texture_type = Texture::Type::CubemapArray}).Create();
    if (!shadow_cubemap_fbo.has_value()) return false;
    m_shadow_cubemap_fbo = std::make_shared<FrameBufferObject>(std::move(shadow_cubemap_fbo.value()));

    int shadow_map_size = kDirectionalShadowMapSizeDefault;
    if (auto scene = SceneManager::GetInstance().GetScene())
    {
        shadow_map_size = scene->GetRenderSettings().GetDirectionalShadowMapSize();
    }
    m_shadow_csm_size_cached = shadow_map_size;
    auto shadow_csm_fbo = FrameBufferObjectBuilder(shadow_map_size, shadow_map_size)
                                        .EnableDepthAttachment({.texture_type = Texture::Type::Texture2DArray, .layers = kDirectionalCascadeMax}).Create();
    if (!shadow_csm_fbo.has_value()) return false;
    m_shadow_csm_fbo = std::make_shared<FrameBufferObject>(std::move(shadow_csm_fbo.value()));

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

    LightManager::GetInstance().AddLightEventListener(this);
    return true;
}

void ForwardRenderPass::Render(ContextState& context_state)
{
    LazyUpdateLightData();
    // Ensure the forward UBO is bound when both forward/deferred exist.
    m_light_ubo->BindBase(0);
    auto scene = SceneManager::GetInstance().GetScene();
    if (!scene) return;
    const auto& settings = scene->GetRenderSettings();
    SyncDirectionalShadowResources(settings);

    if (m_tex_mesh_shader_program != nullptr && m_no_tex_mesh_shader_program != nullptr)
    {
        SCOPED_RENDER_EVENT("ForwardRenderPass");

        // Find the brightest directional light for shadow map rendering, as we only support one directional light shadow map in current implementation
        std::shared_ptr<Light> brightestLight = nullptr;
        float maxIntensity = 0.0f;
        const auto& lights = LightManager::GetInstance().GetLights(Light::Type::Directional);
        for (const auto& [id, weakPtr] : lights) {
            if (auto light = weakPtr.lock()) {
                if (light->GetIntensity() > maxIntensity) {
                    maxIntensity = light->GetIntensity();
                    brightestLight = light;
                }
            }
        }
        RenderDirectionalLightShadow(context_state, brightestLight, settings);
        RenderPointLightShadow(context_state);
        RenderForwardShading(context_state, brightestLight, settings);
    }
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

        auto shadow_cubemap_fbo = FrameBufferObjectBuilder(1024, 1024).EnableDepthAttachment({.texture_type = Texture::Type::CubemapArray, .layers = std::max(light_ubo_data.num_lights, 1u)}).Create();
        m_shadow_cubemap_fbo = std::make_shared<FrameBufferObject>(std::move(shadow_cubemap_fbo.value()));

        m_lighting_data_dirty = false;
    }
}

void ForwardRenderPass::RenderPointLightShadow(ContextState& context_state) const
{
    const auto& lights = LightManager::GetInstance().GetLights(Light::Type::Point);
    if (!lights.empty())
    {
        SCOPED_RENDER_EVENT("Point Light Shadow Pass");
        m_shadow_cubemap_fbo->Bind();
        int light_index = 0;
        for (auto p_light : lights)
        {
            auto light = p_light.second.lock();
            if (!light) continue;
            auto light_owner = light->GetOwner().lock();
    
            SCOPED_RENDER_EVENT(light_owner ? light_owner->GetName() : "light");
            RenderState render_state;
            render_state.depth_stencil_state.depth_test_enabled = true;
            render_state.depth_stencil_state.depth_write_enabled = true;
            context_state.ApplyRenderState(render_state);
            for (int face = 0; face < k_dirs.size(); ++face)
            {
                m_shadow_cubemap_fbo->BindDepthCubemapArray(light_index, face);
                glClear(GL_DEPTH_BUFFER_BIT);
            }
    
            const float near_plane = light->GetCullDistance().x;
            const float far_plane = light->GetCullDistance().y;
            const auto perspective = glm::perspective(glm::radians(90.f), 1.f, near_plane, far_plane);
    
            m_shadow_map_shader_program->Bind();
            m_shadow_map_shader_program->SetUniform("uProjection", perspective);
    
            const auto& meshes = SceneManager::GetInstance().GetMeshes();
            
            // for (auto& p_material : m_mesh_render_materials)
            for (const auto& mesh_it : meshes)
            {
                const auto p_mesh = mesh_it.second.lock();
                if (!p_mesh) continue;
                const auto p_material = p_mesh->m_render_proxy.lock();
                const auto material = std::dynamic_pointer_cast<MeshRenderMaterial>(p_material);
                if (!material) continue;
                
                const auto material_owner = material->GetOwner().lock();
                SCOPED_RENDER_EVENT(material_owner ? material_owner->GetName() : "mesh render material");
    
                const glm::mat4 model = material->GetModelMatrix();
                m_shadow_map_shader_program->SetUniform("uModel", model);
    
                for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
                {
                    material->m_vaos[i]->Bind();
                    // render shadow map for each face of the cubemap
                    for (auto j = 0; j < k_dirs.size(); ++j)
                    {
                        m_shadow_map_shader_program->SetUniform("uView", glm::lookAt(light->GetPosition(), light->GetPosition() + k_dirs[j], k_ups[j]));
                        m_shadow_cubemap_fbo->BindDepthCubemapArray(light_index, j);
                        glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                    }
                    material->m_vaos[i]->Unbind();
                }
            }
            light_index++;
        }
    }
}

void ForwardRenderPass::RenderDirectionalLightShadow(ContextState& context_state, std::shared_ptr<Light> directional_light, const RenderSettings& settings)
{   
    if (!directional_light)
    {
        m_directional_cascades_valid = false;
        return;
    }

    UpdateDirectionalLightCascades(directional_light, settings);
    if (!m_directional_cascades_valid)
        return;

    SCOPED_RENDER_EVENT("Directional Light Shadow Pass");

    RenderState render_state;
    render_state.depth_stencil_state.depth_test_enabled = true;
    context_state.ApplyRenderState(render_state);

    m_shadow_map_shader_program->Bind();

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
        const auto p_material = p_mesh->m_render_proxy.lock();
        const auto material = std::dynamic_pointer_cast<MeshRenderMaterial>(p_material);
        if (!material) continue;
        shadow_items.push_back({material, p_mesh->GetAABB()});
    }

    const int cascade_count = settings.GetDirectionalCascadeCount();
    for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index)
    {
        const std::string cascade_label = "Directional Cascade " + std::to_string(cascade_index);
        SCOPED_RENDER_EVENT(cascade_label);
        m_shadow_csm_fbo->BindDepthTextureLayer(static_cast<unsigned int>(cascade_index));
        glClear(GL_DEPTH_BUFFER_BIT);
        m_shadow_map_shader_program->SetUniform("uProjection", m_directional_cascades[cascade_index].proj);
        m_shadow_map_shader_program->SetUniform("uView", m_directional_cascades[cascade_index].view);

        // World-space cascade bounds for coarse culling of shadow casters.
        const auto& cascade_aabb = m_directional_cascades[cascade_index].world_aabb;
        const bool cull_enabled = cascade_aabb.IsValid();
        for (const auto& item : shadow_items)
        {
            if (cull_enabled && item.aabb.IsValid() && !cascade_aabb.Intersects(item.aabb))
                continue;
            const auto& material = item.material;

            const auto material_owner = material->GetOwner().lock();
            SCOPED_RENDER_EVENT(material_owner ? material_owner->GetName() : "mesh render material");

            const glm::mat4 model = material->GetModelMatrix();
            m_shadow_map_shader_program->SetUniform("uModel", model);

            for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
            {
                material->m_vaos[i]->Bind();
                glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                material->m_vaos[i]->Unbind();
            }
        }
    }
}

void ForwardRenderPass::SyncDirectionalShadowResources(const RenderSettings& settings)
{
    const int shadow_map_size = settings.GetDirectionalShadowMapSize();
    if (shadow_map_size == m_shadow_csm_size_cached)
        return;

    auto shadow_csm_fbo = FrameBufferObjectBuilder(shadow_map_size, shadow_map_size)
                                        .EnableDepthAttachment({.texture_type = Texture::Type::Texture2DArray, .layers = kDirectionalCascadeMax}).Create();
    if (shadow_csm_fbo.has_value())
    {
        m_shadow_csm_fbo = std::make_shared<FrameBufferObject>(std::move(shadow_csm_fbo.value()));
        m_shadow_csm_size_cached = shadow_map_size;
    }
}

void ForwardRenderPass::RenderForwardShading(ContextState& context_state, std::shared_ptr<Light> directional_light, const RenderSettings& settings) const
{
    SCOPED_RENDER_EVENT("Shading Pass");

    m_fbo->Bind();
    glViewport(0, 0, m_viewport_size[0], m_viewport_size[1]);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Only meshes with BVH intersected with view frustum should be rendered
    std::vector<std::shared_ptr<Mesh>> meshes = SceneManager::GetInstance().GetMeshesInViewFrustum();

    // Split meshes into two groups: with textures and without textures
    // This is to minimize shader program switches
    auto mesh_split = std::partition(meshes.begin(), meshes.end(), 
        [](const std::shared_ptr<Mesh>& mesh) {
            return mesh->HasTextures();
        });
    
    auto mesh_shader_program = m_tex_mesh_shader_program.get();
    bool program_uninitialized = true;
    for (const auto& mesh : meshes)
    {
        if (mesh_split != meshes.end() && mesh == *mesh_split)
        {
            mesh_shader_program = m_no_tex_mesh_shader_program.get();
            program_uninitialized = true;
        }

        if (program_uninitialized)
        {
            mesh_shader_program->Bind();
            mesh_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
            mesh_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
            mesh_shader_program->SetUniform("uViewPos", MainCamera::GetInstance().GetPosition());
            if (directional_light && m_directional_cascades_valid)
            {
                const int cascade_count = settings.GetDirectionalCascadeCount();
                for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index)
                {
                    const auto cascade_str = std::to_string(cascade_index);
                    mesh_shader_program->SetUniform("uDirectionalLightOrthoVP[" + cascade_str + "]", m_directional_cascades[cascade_index].vp);
                    mesh_shader_program->SetUniform("uDirectionalLightCascadeSplits[" + cascade_str + "]", m_directional_cascades[cascade_index].split_depth);
                    mesh_shader_program->SetUniform("uDirectionalLightNearFarNorm[" + cascade_str + "]", m_directional_cascades[cascade_index].near_far_norm);
                }
                mesh_shader_program->SetUniform("uDirectionalCascadeCount", cascade_count);
                mesh_shader_program->SetUniform("uDirectionalLightDirection", directional_light->GetDirection());
                mesh_shader_program->SetUniform("uDirectionalLightColor", directional_light->GetColor());
                mesh_shader_program->SetUniform("uDirectionalLightIntensity", directional_light->GetIntensity());
                mesh_shader_program->SetUniform("uEnableDirectionalLightShadow", true);
            }
            else
            {
                mesh_shader_program->SetUniform("uEnableDirectionalLightShadow", false);
            }
            mesh_shader_program->SetUniform("uDirectionalShadowPcfSamples", settings.GetDirectionalShadowPcfSamples());
            mesh_shader_program->SetUniform("uPointShadowPcfSamples", settings.GetPointShadowPcfSamples());
            program_uninitialized = false;
        }

        auto p_material = mesh->m_render_proxy.lock();
        auto material = std::dynamic_pointer_cast<MeshRenderMaterial>(p_material);
        const auto material_owner = material->GetOwner().lock();
        SCOPED_RENDER_EVENT(material_owner ? material_owner->GetName() : "mesh render material");
        if (!material) continue;

        const glm::mat4 model = material->GetModelMatrix();
        mesh_shader_program->SetUniform("uModel", model);

        int current_unit = 1;
        // use shadow map to decide the light contribution
        m_shadow_cubemap_fbo->BindDepthTexture(current_unit++);
        mesh_shader_program->SetUniform("uTexPointLightShadowMaps", current_unit - 1);
        m_shadow_csm_fbo->BindDepthTexture(current_unit++);
        mesh_shader_program->SetUniform("uDirectionalLightTexShadowMap", current_unit - 1);
        int material_texture_unit_begin = current_unit;
        for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
        {
            current_unit = material_texture_unit_begin;
            material->m_vaos[i]->Bind();

            if (material->m_mesh->HasTextures())
            {
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
                mesh_shader_program->SetUniform("uColor", material->m_mesh->m_submeshes[i].m_color);
            }

            glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
            material->m_vaos[i]->Unbind();
        }
    }
}

void ForwardRenderPass::UpdateDirectionalLightCascades(std::shared_ptr<Light> directional_light, const RenderSettings& settings)
{
    m_directional_cascades_valid = false;
    if (!directional_light)
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
        const float log = near_clip * std::pow(ratio, p);
        const float linear = near_clip + clip_range * p;
        splits[cascade_index] = split_lambda * log + (1.0f - split_lambda) * linear;
    }

    const glm::vec3 light_dir = glm::normalize(directional_light->GetDirection());
    const glm::vec3 up_dir = (std::abs(glm::dot(light_dir, glm::vec3(0.f, 1.f, 0.f))) > 0.999f) ? glm::vec3(0.f, 0.f, 1.f) : glm::vec3(0.f, 1.f, 0.f);

    float prev_split = near_clip;
    const float padding_xy = settings.GetDirectionalPaddingXY();
    const float padding_z = settings.GetDirectionalPaddingZ();
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
            glm::vec4( near_width / 2.0f, -near_height / 2.0f, -near_plane, 1.0f),
            glm::vec4( near_width / 2.0f,  near_height / 2.0f, -near_plane, 1.0f),
            glm::vec4(-near_width / 2.0f,  near_height / 2.0f, -near_plane, 1.0f),
            glm::vec4(-far_width / 2.0f, -far_height / 2.0f, -far_plane, 1.0f),
            glm::vec4( far_width / 2.0f, -far_height / 2.0f, -far_plane, 1.0f),
            glm::vec4( far_width / 2.0f,  far_height / 2.0f, -far_plane, 1.0f),
            glm::vec4(-far_width / 2.0f,  far_height / 2.0f, -far_plane, 1.0f)
        };

        glm::vec3 center(0.0f);
        for (auto& corner : corners)
        {
            corner = inv_view * corner;
            center += glm::vec3(corner);
        }
        center /= 8.0f;

        const glm::mat4 light_view = glm::lookAt(center - light_dir, center, up_dir);

        float min_x = std::numeric_limits<float>::max();
        float max_x = std::numeric_limits<float>::lowest();
        float min_y = std::numeric_limits<float>::max();
        float max_y = std::numeric_limits<float>::lowest();
        float min_z = std::numeric_limits<float>::max();
        float max_z = std::numeric_limits<float>::lowest();

        for (const auto& corner : corners)
        {
            const glm::vec4 corner_ls = light_view * corner;
            min_x = std::min(min_x, corner_ls.x);
            max_x = std::max(max_x, corner_ls.x);
            min_y = std::min(min_y, corner_ls.y);
            max_y = std::max(max_y, corner_ls.y);
            min_z = std::min(min_z, corner_ls.z);
            max_z = std::max(max_z, corner_ls.z);
        }

        // XY padding prevents edge casters from popping; Z padding stabilizes depth range.
        min_x -= padding_xy;
        max_x += padding_xy;
        min_y -= padding_xy;
        max_y += padding_xy;
        min_z -= padding_z;
        max_z += padding_z;

        const glm::mat4 light_proj = glm::ortho(min_x, max_x, min_y, max_y, min_z, max_z);
        const AxisAlignedBoundingBox light_aabb(glm::vec3(min_x, min_y, min_z), glm::vec3(max_x, max_y, max_z));
        const glm::mat4 inv_light_view = glm::inverse(light_view);

        m_directional_cascades[cascade_index].view = light_view;
        m_directional_cascades[cascade_index].proj = light_proj;
        m_directional_cascades[cascade_index].vp = light_proj * light_view;
        m_directional_cascades[cascade_index].split_depth = split_dist;
        m_directional_cascades[cascade_index].near_far_norm = max_z - min_z;
        m_directional_cascades[cascade_index].world_aabb = inv_light_view * light_aabb;

        prev_split = split_dist;
    }

    m_directional_cascades_valid = true;
}

} // namespace Aurora
