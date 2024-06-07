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
    virtual void Render(const std::array<int, 2>& viewport_size) = 0;
};
} // namespace Aurora