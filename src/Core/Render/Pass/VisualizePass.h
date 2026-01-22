#pragma once
// std include

// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "glWrapper/Shader.h"
#include "glWrapper/VertexBuffer.h"

namespace Aurora
{

class VisualizePass : public RenderPass
{
public:
    VisualizePass() = default;
    ~VisualizePass() = default;

    bool Init(const std::array<int, 2>& viewport_size) override;
    void Render(ContextState& context_state) override;

private:
    std::shared_ptr<VertexBuffer> m_spatial_hierarchy_vbo;
    std::unique_ptr<ShaderProgram> m_spatial_hierarchy_shader_program;
};
} // namespace Aurora