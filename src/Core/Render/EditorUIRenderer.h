#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Editor/EditorUI.h"

namespace Aurora
{

class EditorUIRenderer
{
public:
    EditorUIRenderer() = default;
    ~EditorUIRenderer();

    bool Init();
    void Render(ContextState& context_state);

    void OnSelectedSceneObjectChange() const;
    bool OnClose() const;

private:
    std::shared_ptr<EditorUI> m_editor_UI;
};
} // namespace Aurora