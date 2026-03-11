#pragma once
// std include
#include <array>

// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Runtime/Scene/RenderSettings.h"
#include "glWrapper/Shader.h"
#include "Runtime/Scene/ILightEventListener.h"

namespace Aurora
{
class Light;
class UniformBuffer;

class ForwardRenderPass : public RenderPass, public ILightEventListener
{
public:
    static constexpr int kDirectionalCascadeMax = RenderSettings::kDirectionalCascadeMax;
    static constexpr int kDirectionalShadowMapSizeDefault = RenderSettings::kDirectionalShadowMapSizeDefault;
    static constexpr float kDirectionalCascadeSplitLambdaDefault = RenderSettings::kDirectionalCascadeSplitLambdaDefault;
    static constexpr float kDirectionalCascadePaddingXYDefault = RenderSettings::kDirectionalCascadePaddingXYDefault;
    static constexpr float kDirectionalCascadePaddingZDefault = RenderSettings::kDirectionalCascadePaddingZDefault;

    ForwardRenderPass();
    ~ForwardRenderPass() = default;

    bool Init(const std::array<int, 2>& viewport_size) override;
    void Render(ContextState& context_state) override;

private:
    struct DirectionalCascadeData
    {
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 proj = glm::mat4(1.0f);
        glm::mat4 vp = glm::mat4(1.0f);
        float split_depth = 0.0f;
        float near_far_norm = 1.0f;
        AxisAlignedBoundingBox world_aabb;
    };

    void RenderPointLightShadow(ContextState& context_state) const;
    void RenderDirectionalLightShadow(ContextState& context_state, std::shared_ptr<Light> directional_light, const RenderSettings& settings);
    void RenderForwardShading(ContextState& context_state, std::shared_ptr<Light> directional_light, const RenderSettings& settings) const;
    void OnLightChanged() override;
    void LazyUpdateLightData();
    void SyncDirectionalShadowResources(const RenderSettings& settings);
    void UpdateDirectionalLightCascades(std::shared_ptr<Light> directional_light, const RenderSettings& settings);

    std::unique_ptr<ShaderProgram> m_tex_mesh_shader_program;
    std::unique_ptr<ShaderProgram> m_no_tex_mesh_shader_program;
    std::unique_ptr<ShaderProgram> m_shadow_map_shader_program;

    std::shared_ptr<FrameBufferObject> m_shadow_cubemap_fbo;
    std::shared_ptr<FrameBufferObject> m_shadow_csm_fbo;
    std::shared_ptr<FrameBufferObject> m_shading_fbo;

    std::unique_ptr<UniformBuffer> m_light_ubo;
    bool m_lighting_data_dirty = true; // lazy update
    bool m_directional_cascades_valid = false;
    std::array<DirectionalCascadeData, kDirectionalCascadeMax> m_directional_cascades;
    int m_shadow_csm_size_cached = kDirectionalShadowMapSizeDefault;
};
} // namespace Aurora
