#pragma once
// std include
#include <array>

// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/Pass/ShadowPass.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "Core/Render/RenderGraph/RenderGraphTexture.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Runtime/Scene/RenderSettings.h"
#include "glWrapper/Shader.h"
#include "glWrapper/Texture.h"
#include "glWrapper/UniformBuffer.h"
#include "Runtime/Scene/ILightEventListener.h"

namespace Aurora
{
class Light;
class FrameBufferObject;
class RenderGraphBuilder;

class DeferredRenderPass : public RenderPass, public ILightEventListener
{
public:
    static constexpr int kDirectionalCascadeMax = RenderSettings::kDirectionalCascadeMax;
    static constexpr int kDirectionalShadowMapSizeDefault = RenderSettings::kDirectionalShadowMapSizeDefault;
    static constexpr float kDirectionalCascadeSplitLambdaDefault = RenderSettings::kDirectionalCascadeSplitLambdaDefault;
    static constexpr float kDirectionalCascadePaddingXYDefault = RenderSettings::kDirectionalCascadePaddingXYDefault;
    static constexpr float kDirectionalCascadePaddingZDefault = RenderSettings::kDirectionalCascadePaddingZDefault;

    DeferredRenderPass() = default;
    ~DeferredRenderPass() = default;

    bool Init() override;

    // Declares the geometry and lighting passes. The G-buffer intermediates are graph
    // resources; the shadow maps come from ShadowPass, which must run first.
    void Render(RenderGraphBuilder& graph_builder,
                RenderGraphTextureHandle color,
                RenderGraphTextureHandle depth,
                const ShadowPass::Result& shadows,
                const std::array<int, 2>& viewport_size);

private:
    void RenderGeometryPass(ContextState& context_state, FrameBufferObject& target) const;
    void RenderLightingPass(ContextState& context_state,
                            const RenderSettings& settings,
                            const ShadowPass::Result& shadows,
                            Texture& point_shadows,
                            Texture& directional_shadows,
                            FrameBufferObject& target,
                            const std::array<Texture*, 5>& gbuffer) const;
    void OnLightChanged() override;
    void LazyUpdateLightData();

    std::unique_ptr<ShaderProgram> m_gbuffer_tex_shader_program;
    std::unique_ptr<ShaderProgram> m_gbuffer_no_tex_shader_program;
    std::unique_ptr<ShaderProgram> m_lighting_shader_program;

    std::unique_ptr<UniformBuffer> m_light_ubo;
    bool m_lighting_data_dirty = true;
};
} // namespace Aurora
