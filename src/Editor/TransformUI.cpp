// std include

// thirdparty include
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/glm.hpp"
// Aurora include
#include "Editor/TransformUI.h"
#include "Runtime/Scene/SceneManager.h"
#include "Editor/DrawUtils.h"

namespace Aurora
{

ComponentLayoutFunction TransformUI::Layout()
{
    return [](const std::string& component_name,
              std::shared_ptr<Component> component, 
              const std::map<std::string, FieldLayoutFunction>& default_field_layouter)->void
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

                std::string rotation_field_name = "m_rotation";
                if (field.first == rotation_field_name)
                {
                    ImGui::PushID(rotation_field_name.c_str());
                    const auto old_vec3 = component->GetField<glm::vec3>(rotation_field_name.c_str());
                    glm::vec3 vec3 = old_vec3;
                    ImGuiTableFlags table_flags = ImGuiTableFlags_None;
                    ImGuiSliderFlags drag_float_flags = ImGuiSliderFlags_AlwaysClamp;
                    if (ImGui::BeginTable("##vec3", 3, table_flags))
                    {
                        // draw color sliders
                        ImGui::TableNextColumn();
                        ImGui::AlignTextToFramePadding();
                        DrawUtils::DrawTextBackground("X", ImVec4(0.793f, 0.148f, 0.0f, 1.0f));
                        ImGui::SameLine();
                        ImGui::DragFloat("##x", &vec3.x, 0.01f, -90.f, 90.f, "%.2f", drag_float_flags);
                        ImGui::TableNextColumn();
                        DrawUtils::DrawTextBackground("Y", ImVec4(0.402f, 0.660f, 0.0f, 1.0f));
                        ImGui::SameLine();
                        ImGui::DragFloat("##y", &vec3.y, 0.01f, -180.f, 180.f, "%.2f", drag_float_flags);
                        ImGui::TableNextColumn();
                        DrawUtils::DrawTextBackground("Z", ImVec4(0.172f, 0.492f, 0.930f, 1.0f));
                        ImGui::SameLine();
                        ImGui::DragFloat("##z", &vec3.z, 0.01f, -180.f, 180.f, "%.2f", drag_float_flags);
                        ImGui::EndTable();

                        if (vec3 != old_vec3)
                        {
                            component->SetField(rotation_field_name.c_str(), vec3);
                            SceneManager::GetInstance().GetScene()->SetDirty();
                        }
                    }
                    ImGui::PopID();
                }
                else
                {
                    if (auto layouter = default_field_layouter.find(field_type); layouter != default_field_layouter.end())
                        layouter->second(field.first, component);
                    else
                        spdlog::error("Unsupported field type {}", field_type.c_str());
                }
                
            }

            ImGui::EndTable();
        }
    };
}
} // namespace Aurora