#pragma once
// std include

// thirdparty include

// Aurora include

class GLFWwindow;
namespace Aurora
{

class RenderPass
{
public:
    RenderPass() = default;
    ~RenderPass() = default;

    virtual bool Init() = 0;
    virtual void Render() = 0;
};
} // namespace Aurora