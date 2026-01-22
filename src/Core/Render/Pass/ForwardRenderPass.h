#pragma once
// std include

// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "glWrapper/Shader.h"

namespace Aurora
{

class ForwardRenderPass : public RenderPass
{
public:
    ForwardRenderPass() = default;
    ~ForwardRenderPass() = default;

    bool Init(const std::array<int, 2>& viewport_size) override;
    void Render(ContextState& context_state) override;

private:
    std::unique_ptr<ShaderProgram> m_point_light_tex_shader_program;
    std::unique_ptr<ShaderProgram> m_point_light_no_tex_shader_program;
    std::unique_ptr<ShaderProgram> m_directional_light_tex_shader_program;
    std::unique_ptr<ShaderProgram> m_directional_light_no_tex_shader_program;
    std::unique_ptr<ShaderProgram> m_shadow_map_shader_program;
    std::unique_ptr<ShaderProgram> m_composite_shader_program;

    std::shared_ptr<FrameBufferObject> m_shadow_cubemap_fbo;
    std::shared_ptr<FrameBufferObject> m_shadow_2Dmap_fbo;
    std::shared_ptr<FrameBufferObject> m_shading_fbo;
};
} // namespace Aurora