#pragma once
// std include
#include <array>
#include <memory>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/RenderGraph/RenderGraphTexture.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Runtime/Scene/RenderSettings.h"

namespace Aurora
{
class Light;
class ShaderProgram;
class RenderGraphBuilder;

class ShadowPass : public RenderPass
{
public:
    static constexpr int kDirectionalCascadeMax = RenderSettings::kDirectionalCascadeMax;
    static constexpr int kDirectionalShadowMapSizeDefault = RenderSettings::kDirectionalShadowMapSizeDefault;

    struct DirectionalCascadeData
    {
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 proj = glm::mat4(1.0f);
        glm::mat4 vp = glm::mat4(1.0f);
        float split_depth = 0.0f;
        float near_far_norm = 1.0f;
        AxisAlignedBoundingBox world_aabb;
    };

    // Produced by Render() and consumed by the shading passes.
    struct Result
    {
        RenderGraphTextureHandle csm;             // Texture2DArray, one layer per cascade.
        RenderGraphTextureHandle point_shadows;   // CubemapArray, one cube per point light.
        std::array<int, 2> resolution = {0, 0};   // Shadow map edge length.
        std::shared_ptr<Light> directional_light; // The light the cascades were fitted to.
        bool cascades_valid = false;
        std::array<DirectionalCascadeData, kDirectionalCascadeMax> cascades;
    };

    ShadowPass() = default;
    ~ShadowPass() = default;

    bool Init() override;

    // Declares the shadow maps and the two passes that fill them. Cascades are fitted here,
    // during setup, so execution only issues draw calls.
    Result Render(RenderGraphBuilder& graph_builder);

private:
    void UpdateDirectionalLightCascades(Result& result, const RenderSettings& settings) const;

    std::unique_ptr<ShaderProgram> m_shadow_map_shader_program;
};
} // namespace Aurora
