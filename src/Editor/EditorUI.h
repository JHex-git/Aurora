#pragma once
// std include

// thirdparty include
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
// Aurora include
#include "Core/Render/WindowUI.h"
#include "Runtime/Scene/SceneObjects/SceneObject.h"

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
    void ShowMainPanel();
    void ShowScenePanel();
    void ShowInspectorPanel();
    void ShowViewPanel();
    void ShowFileContentPanel();

    void ShowDialog();
    void ShowSkyboxDialog();
    void ShowImportMeshDialog();
    void ShowLoadSceneDialog();

    void ShowSceneObjectRecursive(const std::shared_ptr<SceneObject>& scene_object);

    void DrawVec3Control(const std::string& field_name, std::shared_ptr<Component> component);
    void DrawQuaternionControl(const std::string& field_name, std::shared_ptr<Component> component);

    bool m_show_skybox_dialog = false;
    bool m_show_import_mesh_dialog = false;
    bool m_show_load_scene_dialog = false;

    double m_last_frame_time = 0.0f;

    char m_name_buffer[256];
};
} // namespace Aurora