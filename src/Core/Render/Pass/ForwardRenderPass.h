#pragma once
// std include

// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "glWrapper/Shader.h"
#include "Runtime/Scene/ILightEventListener.h"

namespace Aurora
{
class Light;
class UniformBuffer;

class ForwardRenderPass : public RenderPass, public ILightEventListener
{
public:
    ForwardRenderPass();
    ~ForwardRenderPass() = default;

    bool Init(const std::array<int, 2>& viewport_size) override;
    void Render(ContextState& context_state) override;

private:
    void RenderPointLightShadow(ContextState& context_state) const;
    void RenderDirectionalLightShadow(ContextState& context_state, std::shared_ptr<Light> directional_light) const;
    void RenderForwardShading(ContextState& context_state, std::shared_ptr<Light> directional_light) const;
    void OnLightChanged() override;
    void LazyUpdateLightData();

    glm::mat4 GetLightSpaceMatrix(std::shared_ptr<Light> directional_light) const;
    glm::mat4 GetOrthoProjectionMatrix() const;

    std::unique_ptr<ShaderProgram> m_tex_mesh_shader_program;
    std::unique_ptr<ShaderProgram> m_no_tex_mesh_shader_program;
    std::unique_ptr<ShaderProgram> m_shadow_map_shader_program;

    std::shared_ptr<FrameBufferObject> m_shadow_cubemap_fbo;
    std::shared_ptr<FrameBufferObject> m_shadow_2Dmap_fbo;
    std::shared_ptr<FrameBufferObject> m_shading_fbo;

    std::unique_ptr<UniformBuffer> m_light_ubo;
    bool m_lighting_data_dirty = true; // lazy update
};
} // namespace Aurora