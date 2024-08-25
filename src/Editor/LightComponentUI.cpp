// std include
#include <string>
#include <map>
#include <memory>
// thirdparty include
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/glm.hpp"
// Aurora include
#include "Editor/LightComponentUI.h"
#include "Editor/DrawUtils.h"
#include "Runtime/Scene/SceneManager.h"

namespace Aurora
{

ComponentLayoutFunction LightComponentUI::Layout()
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

                std::string color_field_name = "m_color";
                std::string intensity_field_name = "m_intensity";
                if (field.first == color_field_name)
                {
                    ImGui::PushID("m_color");
                    const auto old_vec3 = component->GetField<glm::vec3>(color_field_name.c_str());
                    glm::vec3 vec3 = old_vec3;
                    ImGuiTableFlags table_flags = ImGuiTableFlags_None;
                    ImGuiSliderFlags drag_float_flags = ImGuiSliderFlags_AlwaysClamp;
                    if (ImGui::BeginTable("##vec3", 4, table_flags))
                    {
                        ImGui::TableNextColumn();
                        // visualize light color
                        DrawUtils::DrawRectNoPadding(ImVec4(vec3.x, vec3.y, vec3.z, 1.0f));

                        // draw color sliders
                        ImGui::TableNextColumn();
                        ImVec2 textSize = ImGui::CalcTextSize("z");
                        DrawUtils::DrawTextBackground("r", ImVec4(0.793f, 0.148f, 0.0f, 1.0f));
                        ImGui::SameLine();
                        ImGui::DragFloat("##r", &vec3.x, 0.01f, 0, 1, "%.2f", drag_float_flags);
                        ImGui::TableNextColumn();
                        DrawUtils::DrawTextBackground("g", ImVec4(0.402f, 0.660f, 0.0f, 1.0f));
                        ImGui::SameLine();
                        ImGui::DragFloat("##g", &vec3.y, 0.01f, 0, 1, "%.2f", drag_float_flags);
                        ImGui::TableNextColumn();
                        DrawUtils::DrawTextBackground("b", ImVec4(0.172f, 0.492f, 0.930f, 1.0f));
                        ImGui::SameLine();
                        ImGui::DragFloat("##b", &vec3.z, 0.01f, 0, 1, "%.2f", drag_float_flags);
                        ImGui::EndTable();

                        if (vec3 != old_vec3)
                        {
                            component->SetField(color_field_name.c_str(), vec3);
                            SceneManager::GetInstance().GetScene()->SetDirty();
                        }
                    }
                    ImGui::PopID();
                }
                else if (field.first == intensity_field_name)
                {
                    ImGui::PushID("m_intensity");
                    const auto old_float = component->GetField<float>(intensity_field_name.c_str());
                    float intensity = old_float;
                    if (ImGui::DragFloat("##m_intensity", &intensity, 0.01f, 0, 1000, "%.2f"))
                    {
                        component->SetField(intensity_field_name.c_str(), intensity);
                        SceneManager::GetInstance().GetScene()->SetDirty();
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