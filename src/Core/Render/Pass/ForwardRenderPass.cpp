// std include
#include <algorithm>
// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/ForwardRenderPass.h"
#include "Utility/FileSystem.h"
#include "glWrapper/Shader.h"
#include "glWrapper/RenderEventInfo.h"
#include "Runtime/Scene/Camera.h"
#include "Runtime/Scene/LightManager.h"
#include "Runtime/Scene/TextureManager.h"
#include "glWrapper/Utils.h"
#include "glWrapper/UniformBuffer.h"
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
}

constexpr int MAX_LIGHTS = 3;

struct LightUBO
{
    struct alignas(16) Light
    {
        alignas(16) glm::vec3 light_pos;
        alignas(16) glm::vec3 light_color;
        alignas(16) glm::vec2 cull_distance; // x: near, y: far
        float light_intensity;
    } lights[MAX_LIGHTS];
    alignas(16) unsigned int num_lights;
};

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

    auto shadow_2Dmap_fbo = FrameBufferObjectBuilder(1024, 1024).EnableDepthAttachment({.texture_type = Texture::Type::Texture2D}).Create();
    if (!shadow_2Dmap_fbo.has_value()) return false;
    m_shadow_2Dmap_fbo = std::make_shared<FrameBufferObject>(std::move(shadow_2Dmap_fbo.value()));

    m_light_ubo = std::make_unique<UniformBuffer>(sizeof(LightUBO), 0, GL_STATIC_DRAW);

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
        RenderDirectionalLightShadow(context_state, brightestLight);
        RenderPointLightShadow(context_state);
        RenderForwardShading(context_state, brightestLight);
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
        LightUBO light_ubo_data = {};
        for (const auto& point_light_ptr : point_lights)
        {
            auto light = point_light_ptr.second.lock();
            if (!light) continue;
    
            // fill light data to light_ubo_data
            light_ubo_data.lights[light_ubo_data.num_lights].light_pos = light->GetPosition();
            light_ubo_data.lights[light_ubo_data.num_lights].light_color = light->GetColor();
            light_ubo_data.lights[light_ubo_data.num_lights].light_intensity = light->GetIntensity();
            light_ubo_data.lights[light_ubo_data.num_lights].cull_distance = light->GetCullDistance();
            ++light_ubo_data.num_lights;
        }
        m_light_ubo->SetData(&light_ubo_data, sizeof(LightUBO));

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
            for (int face = 0; face < k_dirs.size(); ++face)
            {
                m_shadow_cubemap_fbo->BindDepthCubemapArray(light_index, face);
                glClear(GL_DEPTH_BUFFER_BIT);
            }
    
            RenderState render_state;
            render_state.depth_stencil_state.depth_test_enabled = true;
            context_state.ApplyRenderState(render_state);
            const auto perspective = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 1000.f);
    
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

void ForwardRenderPass::RenderDirectionalLightShadow(ContextState& context_state, std::shared_ptr<Light> directional_light) const
{   
    if (directional_light)
    {
        SCOPED_RENDER_EVENT("Directional Light Shadow Pass");
        const auto light_owner = directional_light->GetOwner().lock();

        RenderState render_state;
        render_state.depth_stencil_state.depth_test_enabled = true;
        context_state.ApplyRenderState(render_state);
        m_shadow_2Dmap_fbo->Bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        m_shadow_map_shader_program->Bind();
        m_shadow_map_shader_program->SetUniform("uProjection", GetOrthoProjectionMatrix());
        m_shadow_map_shader_program->SetUniform("uView", GetLightSpaceMatrix(directional_light));

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
                glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                material->m_vaos[i]->Unbind();
            }
        }
    }
}

void ForwardRenderPass::RenderForwardShading(ContextState& context_state, std::shared_ptr<Light> directional_light) const
{
    SCOPED_RENDER_EVENT("Shading Pass");

    m_fbo->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
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
            if (directional_light)
            {
                const auto ortho_projection_matrix = GetOrthoProjectionMatrix();
                const auto ortho_matrix = ortho_projection_matrix * GetLightSpaceMatrix(directional_light);
                float near_far_norm = std::abs(2.0f / ortho_projection_matrix[2][2]);
                mesh_shader_program->SetUniform("uDirectionalLightOrthoVP", ortho_matrix);
                mesh_shader_program->SetUniform("uDirectionalLightDirection", directional_light->GetDirection());
                mesh_shader_program->SetUniform("uDirectionalLightColor", directional_light->GetColor());
                mesh_shader_program->SetUniform("uDirectionalLightIntensity", directional_light->GetIntensity());
                mesh_shader_program->SetUniform("uDirectionalLightNearFarNorm", near_far_norm);
                mesh_shader_program->SetUniform("uEnableDirectionalLightShadow", true);
            }
            else
            {
                mesh_shader_program->SetUniform("uEnableDirectionalLightShadow", false);
            }
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
        m_shadow_2Dmap_fbo->BindDepthTexture(current_unit++);
        mesh_shader_program->SetUniform("uDirectionalLightTexShadowMap", current_unit - 1);
        int material_texture_unit_begin = current_unit;
        for (size_t i = 0; i < material->m_mesh->m_submeshes.size(); ++i)
        {
            current_unit = material_texture_unit_begin;
            material->m_vaos[i]->Bind();

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

            glDrawElements(GL_TRIANGLES, material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
            material->m_vaos[i]->Unbind();
        }
    }
}

glm::mat4 ForwardRenderPass::GetOrthoProjectionMatrix() const
{
    const auto perception = 10.f;
    float far_plane = 1000.f;
    const auto ortho_far_plane = far_plane;
    const auto ortho_near_plane = -far_plane;
    const auto ortho = glm::ortho(-perception, perception, -perception, perception, ortho_near_plane, ortho_far_plane);
    return ortho;
}

glm::mat4 ForwardRenderPass::GetLightSpaceMatrix(std::shared_ptr<Light> directional_light) const
{
    const auto light_dir = directional_light->GetDirection();
    glm::vec3 up_dir;
    if (abs(glm::dot(light_dir, glm::vec3(0.f, 1.f, 0.f))) > 0.999f) // avoid up vector parallel to light direction
        up_dir = glm::vec3(0.f, 0.f, 1.f);
    else
        up_dir = glm::normalize(glm::cross(glm::cross(light_dir, glm::vec3(0.f, 1.f, 0.f)), light_dir));
    return glm::lookAt(directional_light->GetPosition(), directional_light->GetPosition() + light_dir, up_dir);
}

} // namespace Aurora