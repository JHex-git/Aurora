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

class EditorUIRenderer
{
public:
    EditorUIRenderer() = default;
    ~EditorUIRenderer();

    bool Init();
    void Render();

private:
    std::vector<std::shared_ptr<WindowUI>> m_window_UIs;
};
} // namespace Aurora