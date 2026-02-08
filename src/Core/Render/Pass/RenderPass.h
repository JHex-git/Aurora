#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "glWrapper/FrameBufferObject.h"
#include "glWrapper/ContextState.h"

struct GLFWwindow;
namespace Aurora
{

class RenderPass
{
public:
    RenderPass() = default;
    ~RenderPass() = default;

    virtual bool Init(const std::array<int, 2>& viewport_size) { m_viewport_size = viewport_size; return true; }
    virtual void Render(ContextState& context_state) = 0;

    std::shared_ptr<FrameBufferObject> GetFrameBuffer() const { return m_fbo; }

protected:
    std::shared_ptr<FrameBufferObject> m_fbo;
    std::array<int, 2> m_viewport_size;
};
} // namespace Aurora