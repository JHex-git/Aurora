#pragma once
// std include
#include <vector>
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/WindowUI.h"

namespace Aurora
{

class EditorUIPass : public RenderPass
{
public:
    EditorUIPass() = default;
    ~EditorUIPass();

    bool Init(GLFWwindow* window) override final;
    void Render() override final;

private:
    std::vector<std::shared_ptr<WindowUI>> m_window_UIs;
    GLFWwindow* m_window = nullptr;
};
} // namespace Aurora