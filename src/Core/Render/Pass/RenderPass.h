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
class RenderGraphBuilder;

class RenderPass
{
public:
    RenderPass() = default;
    ~RenderPass() = default;

    virtual bool Init() { return true; }

protected:
    std::array<int, 2> m_viewport_size;
};
} // namespace Aurora