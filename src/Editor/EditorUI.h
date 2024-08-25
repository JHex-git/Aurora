#pragma once
// std include

// thirdparty include
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
// Aurora include
#include "Runtime/Scene/SceneObjects/SceneObject.h"
#include "Editor/ComponentUI.h"

namespace Aurora
{

class EditorUI
{
public:
    EditorUI() = default;
    ~EditorUI() = default;

    bool Init();
    void Layout();

    void OnSelectedSceneObjectChange();
    bool OnClose();

private:
    void ShowMainPanel();
    void ShowScenePanel();
    void ShowInspectorPanel();
    void ShowViewPanel();
    void ShowFileContentPanel();

    void InitDialogs();
    void InitComponentFieldLayouter();
    void InitComponentLayouter();
    static void DialogAction(bool& show_dialog, const std::function<void()>& confirm_action, const std::function<void()>& cancel_action);

    void ShowSceneObjectRecursive(const std::shared_ptr<SceneObject>& scene_object);

    bool m_show_skybox_dialog = false;
    bool m_show_import_mesh_dialog = false;
    bool m_show_load_scene_dialog = false;
    bool m_show_new_scene_dialog = false;
    bool m_show_save_scene_dialog = false;
    bool m_show_save_scene_as_dialog = false;

    std::map<std::string, std::function<void()>> m_dialog_creator;
    std::map<std::string, ComponentLayoutFunction> m_component_layouter;
    std::map<std::string, FieldLayoutFunction> m_component_field_layouter;

    double m_last_frame_time = 0.0f;

    char m_name_buffer[256];
};
} // namespace Aurora