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
    ShowMainMenu();
    ShowScenePanel();
    ShowInspectorPanel();
    ShowViewPanel();
    ShowFileContentPanel();
}

void EditorUI::ShowMainMenu()
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

    if (!ImGui::Begin("Editor Menu", &m_show_main_menu, window_flags))
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
        ImGui::Text(selected_scene_object->GetName().c_str());
        auto components = selected_scene_object->GetComponents();
        for (auto& component : components)
        {
            std::string component_name = component->GetClassName();

            if (ImGui::CollapsingHeader(component_name.c_str()))
            {
                const auto& fields = ReflectionFactory::GetInstance().GetFields(component_name);
                for (auto& field : fields)
                {
                    ImGui::Text(field.first.c_str());
                    ImGui::SameLine();
                    std::string field_type = field.second.GetFieldType();

                    // TODO: add supported field
                    if (field_type == "std::string")
                        ImGui::Text(component->GetField<std::string>(field.first.c_str()).c_str());
                    else
                    {
                        spdlog::error("Unsupported field type {}", field_type.c_str());
                    }
                }
            }
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

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    auto view_pos = ImGui::GetWindowPos();
    auto view_size = ImGui::GetWindowSize();
    view_pos.y = viewport->Size.y - view_pos.y - view_size.y; // ImGui window pos is from top-left, but OpenGL viewport is from bottom-left
    view_size.y = ImGui::GetContentRegionAvail().y;
    RenderSystem::GetInstance().UpdateViewInfo(ViewInfo{ {static_cast<int>(view_size.x), static_cast<int>(view_size.y)}, 
                                           {static_cast<int>(view_pos.x), static_cast<int>(view_pos.y)} });
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

} // namespace Aurora