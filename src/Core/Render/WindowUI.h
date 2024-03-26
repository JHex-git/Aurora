#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include


class GLFWwindow;
namespace Aurora
{

class WindowUI
{
public:
    virtual bool Init(GLFWwindow* window) = 0;
    virtual void Layout() = 0;
};
} // namespace Aurora