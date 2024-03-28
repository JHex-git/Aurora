#pragma once
// std include

// thirdparty include
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
// Aurora include
#include "Core/Render/WindowUI.h"
#include "Runtime/Scene/SceneObject.h"

namespace Aurora
{

class EditorUI : public WindowUI
{
public:
    EditorUI() = default;
    ~EditorUI() = default;

    bool Init() override final;
    void Layout() override final;

private:
    void ShowMainMenu();
    void ShowScenePanel();
    void ShowInspectorPanel();
    void ShowViewPanel();
    void ShowFileContentPanel();

    void ShowSceneObjectRecursive(const std::shared_ptr<SceneObject>& scene_object);

    bool m_show_main_menu = true;
    bool m_show_scene_panel = true;
    bool m_show_inspector_panel = true;
};
} // namespace Aurora