// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_internal.h"
#include "thirdparty/imgui/backends/imgui_impl_glfw.h"
#include "thirdparty/imgui/backends/imgui_impl_opengl3.h"
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Editor/EditorUI.h"
#include "Editor/LightComponentUI.h"
#include "Editor/TransformUI.h"
#include "Editor/DrawUtils.h"
#include "Core/Render/RenderSystem.h"
#include "Runtime/Scene/SceneManager.h"
#include "Utility/FileSystem.h"

namespace Aurora
{

bool EditorUI::Init()
{
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.Fonts->AddFontFromFileTTF(FileSystem::GetFullPath("assets/fonts/Microsoft Yahei.ttf").c_str(), 17.0f);

    ImGuiStyle* style  = &ImGui::GetStyle();
    ImVec4*     colors = style->Colors;
    auto titlebg_color = ImVec4(42.0f / 255.0f, 45.0f / 255.0f, 46.0f / 255.0f, 1.00f);
    colors[ImGuiCol_Tab]                   = titlebg_color;
    colors[ImGuiCol_TabHovered]            = titlebg_color;
    colors[ImGuiCol_TabActive]             = titlebg_color;
    colors[ImGuiCol_TabUnfocused]          = titlebg_color;
    colors[ImGuiCol_TabUnfocusedActive]    = titlebg_color;
    colors[ImGuiCol_TitleBg]               = titlebg_color;
    colors[ImGuiCol_TitleBgActive]         = titlebg_color;
    colors[ImGuiCol_TitleBgCollapsed]      = titlebg_color;
    colors[ImGuiCol_ButtonHovered]         = ImVec4(100.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.00f);

    InitDialogs();
    InitComponentFieldLayouter();
    InitComponentLayouter();
    return true;
}

void EditorUI::InitDialogs()
{
    m_dialog_creator["Skybox"] = [this]() { 
        if (m_show_skybox_dialog)
        {
            static char right[256] = "assets/textures/skybox/right.jpg";
            static char left[256] = "assets/textures/skybox/left.jpg";
            static char top[256] = "assets/textures/skybox/top.jpg";
            static char bottom[256] = "assets/textures/skybox/bottom.jpg";
            static char front[256] = "assets/textures/skybox/front.jpg";
            static char back[256] = "assets/textures/skybox/back.jpg";

            ImGui::OpenPopup("Skybox");
            if (ImGui::BeginPopupModal("Skybox", &m_show_skybox_dialog, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
            {
                
                if (ImGui::BeginTable("##TableSkyboxDialog", 2))
                {
                    float first_column_width = ImGui::CalcTextSize("Bottom").x;
                    ImGui::TableSetupColumn("##ColumnSkyboxDialog1", ImGuiTableColumnFlags_WidthFixed, first_column_width);
                    ImGui::TableSetupColumn("##ColumnSkyboxDialog2", ImGuiTableColumnFlags_WidthFixed, 300.f);

                    ImGui::TableNextColumn();
                    ImGui::Text("Right");
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::InputText("##RightSkybox", right, IM_ARRAYSIZE(right), ImGuiInputTextFlags_None);
                    ImGui::TableNextColumn();
                    ImGui::Text("Left");
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::InputText("##LeftSkybox", left, IM_ARRAYSIZE(left), ImGuiInputTextFlags_None);
                    ImGui::TableNextColumn();
                    ImGui::Text("Top");
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::InputText("##TopSkybox", top, IM_ARRAYSIZE(top), ImGuiInputTextFlags_None);
                    ImGui::TableNextColumn();
                    ImGui::Text("Bottom");
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::InputText("##BottomSkybox", bottom, IM_ARRAYSIZE(bottom), ImGuiInputTextFlags_None);
                    ImGui::TableNextColumn();
                    ImGui::Text("Front");
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::InputText("##FrontSkybox", front, IM_ARRAYSIZE(front), ImGuiInputTextFlags_None);
                    ImGui::TableNextColumn();
                    ImGui::Text("Back");
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::InputText("##BackSkybox", back, IM_ARRAYSIZE(back), ImGuiInputTextFlags_None);

                    ImGui::EndTable();
                }

                DialogAction(m_show_skybox_dialog, 
                    [this](){ SceneManager::GetInstance().GetScene()->AddSkybox({right, left, top, bottom, front, back}); }, 
                    [](){});
                ImGui::EndPopup();
            }
        }
    };

    m_dialog_creator["Import Mesh"] = [this]() {
        if (m_show_import_mesh_dialog)
        {
            ImGui::OpenPopup("Import Mesh");
            if (ImGui::BeginPopupModal("Import Mesh", &m_show_import_mesh_dialog, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
            {
                static char file_path[256] = "assets/models/";
                ImGui::Text("File Path");
                ImGui::SameLine();
                ImGui::InputText("##Mesh File Path", file_path, IM_ARRAYSIZE(file_path));
                DialogAction(m_show_import_mesh_dialog, 
                    [this](){ 
                        auto scene = SceneManager::GetInstance().GetScene();
                        if (scene) scene->LoadMesh(file_path); 
                    }, 
                    [](){});
                ImGui::EndPopup();
            }
        }
    };

    m_dialog_creator["Load Scene"] = [this](){
        if (m_show_load_scene_dialog)
        {
            ImGui::OpenPopup("Load Scene");
            if (ImGui::BeginPopupModal("Load Scene", &m_show_load_scene_dialog, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
            {
                static char file_path[256] = "samples/test.xml";
                ImGui::Text("Scene Path");
                ImGui::SameLine();
                ImGui::InputText("##Scene File Path", file_path, IM_ARRAYSIZE(file_path));
                DialogAction(m_show_load_scene_dialog, 
                    [this](){ SceneManager::GetInstance().LoadScene(FileSystem::GetFullPath(file_path)); }, 
                    [](){});
                ImGui::EndPopup();
            }
        }
    };

    m_dialog_creator["New Scene"] = [this](){
        if (m_show_new_scene_dialog)
        {
            ImGui::OpenPopup("New Scene");
            if (ImGui::BeginPopupModal("New Scene", &m_show_new_scene_dialog, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
            {
                static char file_path[256] = "samples/new.xml";
                ImGui::Text("Save Path");
                ImGui::SameLine();
                ImGui::InputText("##Scene File Path", file_path, IM_ARRAYSIZE(file_path));
                DialogAction(m_show_new_scene_dialog, 
                    [](){ SceneManager::GetInstance().CreateNewScene(FileSystem::GetFullPath(file_path)); }, [](){});
                ImGui::EndPopup();
            }
        }
    };

    m_dialog_creator["Save Scene"] = [this](){
        if (m_show_save_scene_dialog)
        {
            ImGui::OpenPopup("Save Scene");
            if (ImGui::BeginPopupModal("Save Scene", &m_show_save_scene_dialog, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
            {
                ImGui::Text("Scene has been modified, do you want to save it?");
                DialogAction(m_show_save_scene_dialog, 
                    [this](){ SceneManager::GetInstance().SaveScene(); }, [](){ SceneManager::GetInstance().GetScene()->ClearDirty(); });
                ImGui::EndPopup();
            }
        }
    };

    m_dialog_creator["Save Scene As"] = [this](){
        if (m_show_save_scene_as_dialog)
        {
            ImGui::OpenPopup("Save Scene As");
            if (ImGui::BeginPopupModal("Save Scene As", &m_show_save_scene_as_dialog, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
            {
                static char file_path[256] = "samples/new.xml";
                ImGui::Text("Scene Path");
                ImGui::SameLine();
                ImGui::InputText("##Scene File Path", file_path, IM_ARRAYSIZE(file_path));
                DialogAction(m_show_save_scene_as_dialog, 
                    [this](){ SceneManager::GetInstance().SaveSceneAs(FileSystem::GetFullPath(file_path)); }, [](){});
                ImGui::EndPopup();
            }
        }
    };
}

void EditorUI::InitComponentFieldLayouter()
{
    // TODO: add supported field
    m_component_field_layouter["std::string"] = [](const std::string& field_name, std::shared_ptr<Component> component) {
        ImGui::Text(component->GetField<std::string>(field_name).c_str());
    };

    m_component_field_layouter["glm::vec3"] = [](const std::string& field_name, std::shared_ptr<Component> component) {
        ImGui::PushID(field_name.c_str());
        const auto old_vec3 = component->GetField<glm::vec3>(field_name.c_str());
        glm::vec3 vec3 = old_vec3;
        ImGuiTableFlags table_flags = ImGuiTableFlags_None;
        ImGuiSliderFlags drag_float_flags = ImGuiSliderFlags_AlwaysClamp;
        if (ImGui::BeginTable("##vec3", 3, table_flags))
        {
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            DrawUtils::DrawTextBackground("X", ImVec4(0.793f, 0.148f, 0.0f, 1.0f));
            ImGui::SameLine();
            ImGui::DragFloat("##x", &vec3.x, 0.1f, -FLT_MAX, FLT_MAX, "%.2f", drag_float_flags);
            ImGui::TableNextColumn();
            DrawUtils::DrawTextBackground("Y", ImVec4(0.402f, 0.660f, 0.0f, 1.0f));
            ImGui::SameLine();
            ImGui::DragFloat("##y", &vec3.y, 0.1f, -FLT_MAX, FLT_MAX, "%.2f", drag_float_flags);
            ImGui::TableNextColumn();
            DrawUtils::DrawTextBackground("Z", ImVec4(0.172f, 0.492f, 0.930f, 1.0f));
            ImGui::SameLine();
            ImGui::DragFloat("##z", &vec3.z, 0.1f, -FLT_MAX, FLT_MAX, "%.2f", drag_float_flags);
            ImGui::EndTable();

            if (vec3 != old_vec3)
            {
                component->SetField(field_name.c_str(), vec3);
                SceneManager::GetInstance().GetScene()->SetDirty();
            }
        }
        ImGui::PopID();
    };

    m_component_field_layouter["glm::quat"] = [](const std::string& field_name, std::shared_ptr<Component> component) {
        // FIXME: multiple member value change caused by precision
        ImGui::PushID(field_name.c_str());
        const auto old_quat = component->GetField<glm::quat>(field_name.c_str());
        auto angle = glm::eulerAngles(old_quat);// radius
        angle = glm::degrees(angle); // degree
        ImGuiTableFlags table_flags = ImGuiTableFlags_None;
        ImGuiSliderFlags drag_float_flags = ImGuiSliderFlags_AlwaysClamp;
        if (ImGui::BeginTable("##quat", 3, table_flags))
        {
            ImGui::TableNextColumn();
            DrawUtils::DrawTextBackground("x", ImVec4(0.793f, 0.148f, 0.0f, 1.0f));
            ImGui::SameLine();
            ImGui::DragFloat("##x", &angle.x, 0.1f, 0, 359.9, "%.2f", drag_float_flags);
            ImGui::TableNextColumn();
            DrawUtils::DrawTextBackground("y", ImVec4(0.402f, 0.660f, 0.0f, 1.0f));
            ImGui::SameLine();
            ImGui::DragFloat("##y", &angle.y, 0.1f, 0, 359.9, "%.2f", drag_float_flags);
            ImGui::TableNextColumn();
            DrawUtils::DrawTextBackground("z", ImVec4(0.172f, 0.492f, 0.930f, 1.0f));
            ImGui::SameLine();
            ImGui::DragFloat("##z", &angle.z, 0.1f, 0, 359.9, "%.2f", drag_float_flags);
            ImGui::EndTable();

            const auto new_quat = glm::quat(glm::radians(angle));
            if (glm::angle(glm::inverse(old_quat) * new_quat) > 1e-5f) // 0.01 radian threshold
            {
                component->SetField(field_name.c_str(), new_quat);
                SceneManager::GetInstance().GetScene()->SetDirty();
            }
        }
        ImGui::PopID();
    };

    m_component_field_layouter["float"] = [](const std::string& field_name, std::shared_ptr<Component> component) {
        ImGui::PushID(field_name.c_str());
        const auto old_float = component->GetField<float>(field_name.c_str());
        float value = old_float;
        if (ImGui::DragFloat("##float", &value, 0.01f))
        {
            if (value != old_float)
            {
                component->SetField(field_name.c_str(), value);
                SceneManager::GetInstance().GetScene()->SetDirty();
            }
        }
        ImGui::PopID();
    };
}

void EditorUI::InitComponentLayouter()
{
    m_component_layouter["default"] = [this](const std::string& component_name, std::shared_ptr<Component> component, 
                                            const std::map<std::string, FieldLayoutFunction>& default_field_layouter) {
        std::string table_name = "##Table" + component_name;
        if (ImGui::BeginTable(table_name.c_str(), 2))
        {
            const auto& fields = ReflectionFactory::GetInstance().GetFields(component_name);
            for (auto& field : fields)
            {

                ImGui::TableNextColumn();
                ImGui::Text(field.first.c_str());
                ImGui::TableNextColumn();
                std::string field_type = field.second.GetFieldType();

                auto layouter = m_component_field_layouter.find(field_type);
                if (layouter != m_component_field_layouter.end())
                    layouter->second(field.first, component);
                else
                    spdlog::error("Unsupported field type {}", field_type.c_str());
            }

            ImGui::EndTable();
        }
    };

    m_component_layouter["Light"] = LightComponentUI().Layout();
    m_component_layouter["Transform"] = TransformUI().Layout();
}

void EditorUI::Layout()
{
    ShowMainPanel();
    ShowScenePanel();
    ShowInspectorPanel();
    ShowViewPanel();
    ShowFileContentPanel();

    // Show dialog
    for (auto& dialog : m_dialog_creator)
    {
        dialog.second();
    }
}

void EditorUI::ShowMainPanel()
{
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoWindowMenuButton;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | 
                                    ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); // ensure no padding between panels

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowViewport(viewport->ID);

    if (!ImGui::Begin("Editor", nullptr, window_flags))
    {
        ImGui::End();
        return;
    }

    ImGuiID dockspace_id = ImGui::GetID("Editor Main Dock");
    if (!ImGui::DockBuilderGetNode(dockspace_id))
    {
        ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);
        
        ImGuiID left_container;
        ImGuiID right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &left_container);

        ImGuiID left_top_container;
        ImGuiID left_container_down = ImGui::DockBuilderSplitNode(left_container, ImGuiDir_Down, 0.25f, nullptr, &left_top_container);

        ImGuiID left_top_container_left;
        ImGuiID left_top_container_right = ImGui::DockBuilderSplitNode(left_top_container, ImGuiDir_Right, 0.75f, nullptr, &left_top_container_left);

        ImGui::DockBuilderDockWindow("Scene", left_top_container_left);
        ImGui::DockBuilderDockWindow("View", left_top_container_right);
        ImGui::DockBuilderDockWindow("File Content", left_container_down);
        ImGui::DockBuilderDockWindow("Inspector", right);
    }
    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_NoWindowMenuButton);
    
    ImGui::PopStyleVar();
    
    bool has_scene = SceneManager::GetInstance().GetScene() != nullptr;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            auto scene = SceneManager::GetInstance().GetScene();
            bool scene_dirty = scene ? scene->IsDirty() : false;
            if (ImGui::MenuItem("Load Scene..."))
            {
                if (scene_dirty) m_show_save_scene_dialog = true;
                else m_show_load_scene_dialog = true;
            }
            if (ImGui::MenuItem("New Scene"))
            {
                if (scene_dirty) m_show_save_scene_dialog = true;
                else m_show_new_scene_dialog = true;
            }
            if (ImGui::MenuItem("Save Scene", nullptr, nullptr, has_scene))
            {
                SceneManager::GetInstance().SaveScene();
            }
            if (ImGui::MenuItem("Save Scene As...", nullptr, nullptr, has_scene))
            {
                m_show_save_scene_as_dialog = true;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene"))
        {
            if (ImGui::MenuItem("New Scene Object", nullptr, nullptr, has_scene))
            {
                SceneManager::GetInstance().GetScene()->CreateEmptySceneObject();
            }

            if (ImGui::BeginMenu("New Light", has_scene))
            {
                if (ImGui::MenuItem("Point Light"))
                {
                    SceneManager::GetInstance().GetScene()->AddPointLight();
                }
                if (ImGui::MenuItem("Directional Light"))
                {
                    SceneManager::GetInstance().GetScene()->AddDirectionalLight();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Load Mesh", has_scene))
            {
                auto scene = SceneManager::GetInstance().GetScene();
                if (ImGui::MenuItem("Sphere"))
                {
                    if (scene) scene->LoadMesh("assets/models/sphere.obj");
                }
                if (ImGui::MenuItem("Cube"))
                {
                    if (scene) scene->LoadMesh("assets/models/cube.obj");
                }
                if (ImGui::MenuItem("From file..."))
                {
                    m_show_import_mesh_dialog = true;
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Add Skybox...", nullptr, nullptr, has_scene))
            {
                m_show_skybox_dialog = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::End();
}

void EditorUI::ShowSceneObjectRecursive(const std::shared_ptr<SceneObject>& scene_object)
{
    auto children = scene_object->GetChildren();
    ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (children.empty()) tree_node_flags |= ImGuiTreeNodeFlags_Leaf;
    if (scene_object == SceneManager::GetInstance().GetScene()->GetSelectedSceneObject())
        tree_node_flags |= ImGuiTreeNodeFlags_Selected;

    bool is_open = ImGui::TreeNodeEx(scene_object->GetName().c_str(), tree_node_flags);
    if (ImGui::IsItemClicked())
    {
        SceneManager::GetInstance().GetScene()->SetSelectedSceneObject(scene_object);
    }

    if (is_open)
    {
        for (auto& child : children)
        {
            ShowSceneObjectRecursive(child);
        }
        ImGui::TreePop();
    }

}

void EditorUI::OnSelectedSceneObjectChange()
{
    strcpy(m_name_buffer, SceneManager::GetInstance().GetScene()->GetSelectedSceneObject()->GetName().c_str());
}

bool EditorUI::OnClose()
{
    auto scene = SceneManager::GetInstance().GetScene();
    if (scene && scene->IsDirty())
    {
        m_show_save_scene_dialog = true;
        return false;
    }
    return true;
}

void EditorUI::ShowScenePanel()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove;
    if (!ImGui::Begin("Scene", nullptr, window_flags))
    {
        ImGui::End();
        return;
    }
    auto scene = SceneManager::GetInstance().GetScene();
    if (scene)
    {
        auto scene_objects = scene->GetSceneObjects();
        for (auto& scene_object : scene_objects)
        {
            ShowSceneObjectRecursive(scene_object);
        }
    }
    ImGui::End();
}

void EditorUI::ShowInspectorPanel()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove;
    if (!ImGui::Begin("Inspector", nullptr, window_flags))
    {
        ImGui::End();
        return;
    }

    if (auto scene = SceneManager::GetInstance().GetScene())
    {
        auto selected_scene_object = scene->GetSelectedSceneObject();
        if (selected_scene_object)
        {
            ImGui::PushID(selected_scene_object->GetName().c_str());
            ImGui::Text("Name");
            ImGui::SameLine();
            if (ImGui::InputText("##Name", m_name_buffer, IM_ARRAYSIZE(m_name_buffer), 
                ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_EnterReturnsTrue, 
                [](ImGuiInputTextCallbackData* data) -> int {
                    auto selected_scene_object = SceneManager::GetInstance().GetScene()->GetSelectedSceneObject();
                    selected_scene_object->SetName(data->Buf);
                    return 0;
                }))
            {
                auto selected_scene_object = SceneManager::GetInstance().GetScene()->GetSelectedSceneObject();
                selected_scene_object->SetName(m_name_buffer);
            }

            auto components = selected_scene_object->GetComponents();
            for (unsigned int i = 0; i <= components.size(); ++i)
            {
                std::shared_ptr<Component> component = (i == 0) ? selected_scene_object->GetTransform() : components[i - 1];
                std::string component_name = component->GetClassReflectName();

                if (ImGui::CollapsingHeader(component_name.c_str()))
                {
                    if (auto layout = m_component_layouter.find(component_name); layout != m_component_layouter.end())
                        layout->second(component_name, component, m_component_field_layouter);
                    else
                    {
                        m_component_layouter["default"](component_name, component, m_component_field_layouter);
                    }
                }
            }
            ImGui::PopID();
        }
    }
    
    ImGui::End();
}

void EditorUI::ShowViewPanel()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove;
    if (!ImGui::Begin("View", nullptr, window_flags))
    {
        ImGui::End();
        return;
    }

    double current_time = static_cast<float>(glfwGetTime());
    double delta_time = current_time - m_last_frame_time;
    m_last_frame_time = current_time;
    float fps = 1.0f / delta_time;
    ImGui::Text("FPS: %.1f", fps);
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    auto view_pos = ImGui::GetCursorScreenPos(); // left top corner
    auto view_size = ImGui::GetContentRegionAvail();
    RenderSystem::GetInstance().UpdateViewInfo(ViewInfo{ {static_cast<int>(view_size.x), static_cast<int>(view_size.y)}, 
                                           {static_cast<int>(view_pos.x), static_cast<int>(view_pos.y)} });
    ImGui::Image((void*)(intptr_t)RenderSystem::GetInstance().GetRenderTargetID(), ImVec2(view_size.x, view_size.y), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}

void EditorUI::ShowFileContentPanel()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove;
    if (!ImGui::Begin("File Content", nullptr, window_flags))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("File Content Panel");
    ImGui::End();
}

void EditorUI::DialogAction(bool& show_dialog, const std::function<void()>& confirm_action, const std::function<void()>& cancel_action)
{
    const auto& style = ImGui::GetStyle();
    float confirmButtonWidth = ImGui::CalcTextSize("Confirm").x + style.FramePadding.x * 2.f;
    float cancelButtonWidth = ImGui::CalcTextSize("Cancel").x + style.FramePadding.x * 2.f;
    float widthNeeded = confirmButtonWidth + cancelButtonWidth + style.ItemSpacing.x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);
    if (ImGui::Button("Confirm##Dialog"))
    {
        show_dialog = false;
        confirm_action();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel##Dialog"))
    {
        show_dialog = false;
        cancel_action();
    }
}
} // namespace Aurora