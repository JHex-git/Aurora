#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"

namespace Aurora
{

class ShaderProgram;

class TonemapPass : public RenderPass
{
public:
    TonemapPass() = default;
    ~TonemapPass() = default;

    bool Init(const std::array<int, 2>& viewport_size) override;
    void Render(ContextState& context_state) override;

    void SetSourceFrameBuffer(std::shared_ptr<FrameBufferObject> source) { m_source_fbo = std::move(source); }

private:
    std::shared_ptr<FrameBufferObject> m_source_fbo;
    std::unique_ptr<ShaderProgram> m_shader_program;
};
} // namespace Aurora
