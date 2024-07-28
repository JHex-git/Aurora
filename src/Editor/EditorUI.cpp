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
#include "Core/Render/RenderSystem.h"
#include "Runtime/Scene/SceneManager.h"

namespace Aurora
{
bool EditorUI::Init()
{
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    return true;
}

void EditorUI::Layout()
{
    ShowMainPanel();
    ShowScenePanel();
    ShowInspectorPanel();
    ShowViewPanel();
    ShowFileContentPanel();

    ShowDialog();
}

void EditorUI::ShowDialog()
{
    ShowSkyboxDialog();
}

void EditorUI::ShowMainPanel()
{
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_DockSpace;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | 
                                    ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize); // FIXME:
    ImGui::SetNextWindowViewport(viewport->ID);

    if (!ImGui::Begin("Editor", &m_show_main_panel, window_flags))
    {
        ImGui::End();
        return;
    }

    ImGuiID dockspace_id = ImGui::GetID("Editor Main Dock");
    if (!ImGui::DockBuilderGetNode(dockspace_id))
    {
        ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize); // FIXME:
        
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
    ImGui::DockSpace(dockspace_id);
    
    
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Load Scene"))
            {
                // TODO:
                spdlog::info("Load Scene");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene"))
        {
            if (ImGui::MenuItem("New Scene Object"))
            {
                // TODO:
            }

            if (ImGui::BeginMenu("Load Mesh"))
            {
                if (ImGui::MenuItem("Sphere"))
                {
                    SceneManager::GetInstance().LoadMesh("assets/models/sphere.obj");
                }
                if (ImGui::MenuItem("Cube"))
                {
                    SceneManager::GetInstance().LoadMesh("assets/models/cube.obj");
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Add Skybox"))
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

void EditorUI::ShowScenePanel()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove;
    if (!ImGui::Begin("Scene", &m_show_scene_panel, window_flags))
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
    if (!ImGui::Begin("Inspector", &m_show_inspector_panel, window_flags))
    {
        ImGui::End();
        return;
    }
    auto selected_scene_object = SceneManager::GetInstance().GetScene()->GetSelectedSceneObject();
    if (selected_scene_object)
    {
        ImGui::PushID(selected_scene_object->GetName().c_str());
        ImGui::Text(selected_scene_object->GetName().c_str());

        auto components = selected_scene_object->GetComponents();
        for (unsigned int i = 0; i <= components.size(); ++i)
        {
            std::shared_ptr<Component> component = (i == 0) ? selected_scene_object->GetTransform() : components[i - 1]; // FIXME: double free
            std::string component_name = component->GetClassReflectName();

            if (ImGui::CollapsingHeader(component_name.c_str()))
            {
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

                        // TODO: add supported field
                        if (field_type == "std::string")
                            ImGui::Text(component->GetField<std::string>(field.first.c_str()).c_str());
                        else if (field_type == "glm::vec3")
                        {
                            DrawVec3Control(field.first, component);
                        }
                        else if (field_type == "glm::quat")
                        {
                            DrawQuaternionControl(field.first, component);
                        }
                        else
                        {
                            spdlog::error("Unsupported field type {}", field_type.c_str());
                        }
                    }

                    ImGui::EndTable();
                }
            }
        }
        ImGui::PopID();
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
    double delta_time = current_time - last_frame_time;
    last_frame_time = current_time;
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

void EditorUI::DrawVec3Control(const std::string& field_name, std::shared_ptr<Component> component)
{
    ImGui::PushID(field_name.c_str());
    const auto old_vec3 = component->GetField<glm::vec3>(field_name.c_str());
    glm::vec3 vec3 = old_vec3;
    ImGuiTableFlags table_flags = ImGuiTableFlags_None;
    ImGuiSliderFlags drag_float_flags = ImGuiSliderFlags_AlwaysClamp;
    if (ImGui::BeginTable("##vec3", 3, table_flags))
    {
        ImGui::TableNextColumn();
        ImGui::Text("x");
        ImGui::SameLine();
        ImGui::DragFloat("##x", &vec3.x, 0.1f, -FLT_MAX, FLT_MAX, "%.2f", drag_float_flags);
        ImGui::TableNextColumn();
        ImGui::Text("y");
        ImGui::SameLine();
        ImGui::DragFloat("##y", &vec3.y, 0.1f, -FLT_MAX, FLT_MAX, "%.2f", drag_float_flags);
        ImGui::TableNextColumn();
        ImGui::Text("z");
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
}

void EditorUI::DrawQuaternionControl(const std::string& field_name, std::shared_ptr<Component> component)
{
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
        ImGui::Text("x");
        ImGui::SameLine();
        ImGui::DragFloat("##x", &angle.x, 0.1f, 0, 359.9, "%.2f", drag_float_flags);
        ImGui::TableNextColumn();
        ImGui::Text("y");
        ImGui::SameLine();
        ImGui::DragFloat("##y", &angle.y, 0.1f, 0, 359.9, "%.2f", drag_float_flags);
        ImGui::TableNextColumn();
        ImGui::Text("z");
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
}

void EditorUI::ShowSkyboxDialog()
{
    if (m_show_skybox_dialog)
    {
        static char right[256] = "assets/textures/skybox/right.jpg";
        static char left[256] = "assets/textures/skybox/left.jpg";
        static char top[256] = "assets/textures/skybox/top.jpg";
        static char bottom[256] = "assets/textures/skybox/bottom.jpg";
        static char front[256] = "assets/textures/skybox/front.jpg";
        static char back[256] = "assets/textures/skybox/back.jpg";


        ImGui::OpenPopup("Skybox Dialog");
        if (ImGui::BeginPopupModal("Skybox Dialog", &m_show_skybox_dialog, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
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
                ImGui::InputText("##RightSkybox", right, sizeof(right), ImGuiInputTextFlags_None);
                ImGui::TableNextColumn();
                ImGui::Text("Left");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##LeftSkybox", left, sizeof(left), ImGuiInputTextFlags_None);
                ImGui::TableNextColumn();
                ImGui::Text("Top");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##TopSkybox", top, sizeof(top), ImGuiInputTextFlags_None);
                ImGui::TableNextColumn();
                ImGui::Text("Bottom");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##BottomSkybox", bottom, sizeof(bottom), ImGuiInputTextFlags_None);
                ImGui::TableNextColumn();
                ImGui::Text("Front");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##FrontSkybox", front, sizeof(front), ImGuiInputTextFlags_None);
                ImGui::TableNextColumn();
                ImGui::Text("Back");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##BackSkybox", back, sizeof(back), ImGuiInputTextFlags_None);

                ImGui::EndTable();
            }

            const auto& style = ImGui::GetStyle();
            float confirmButtonWidth = ImGui::CalcTextSize("Confirm").x + style.FramePadding.x * 2.f;
            float cancelButtonWidth = ImGui::CalcTextSize("Cancel").x + style.FramePadding.x * 2.f;
            float widthNeeded = confirmButtonWidth + cancelButtonWidth + style.ItemSpacing.x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);
            if (ImGui::Button("Confirm##SkyboxDialog"))
            {
                m_show_skybox_dialog = false;
                SceneManager::GetInstance().GetScene()->AddSkybox({right, left, top, bottom, front, back});
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##SkyboxDialog"))
            {
                m_show_skybox_dialog = false;
            }
            ImGui::EndPopup();
        }
    }
}
} // namespace Aurora