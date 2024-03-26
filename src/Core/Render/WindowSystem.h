#pragma once
#include <string>

struct GLFWwindow;
namespace Aurora
{

class WindowSystem
{
public:
    WindowSystem() = default;
    ~WindowSystem();

    bool Init(unsigned int width, unsigned int height, std::string title);
    void Shutdown() const;

    void SwapBuffers() const;
    void Clear() const;

    void PollEvents() const;

    bool ShouldClose() const;

    GLFWwindow* GetWindow() const { return m_window; }

private:
    unsigned int m_width;
    unsigned int m_height;
    std::string m_title;
    GLFWwindow* m_window;
};
} // namespace Aurora
