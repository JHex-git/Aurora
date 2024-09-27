// std include

// thirdparty include
#include "thirdparty/imgui/imgui_internal.h"
// Aurora include
#include "Editor/DrawUtils.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
namespace Aurora
{

void DrawUtils::DrawTextBackground(const std::string& text, const ImVec4& color)
{
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 beginPos = ImGui::GetCursorScreenPos();
    beginPos.x -= ImGui::GetStyle().FramePadding.x;
    
    ImVec2 endPos = ImVec2(beginPos.x + textSize.x + ImGui::GetStyle().FramePadding.x * 2, beginPos.y + ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2);
    ImRect bb(beginPos, endPos);
    ImGui::GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(color));
    ImGui::Text(text.c_str());
}

void DrawUtils::DrawRectNoPadding(const ImVec4& color)
{
    ImVec2 begin_pos = ImGui::GetCursorScreenPos();
    auto item_size = ImGui::GetItemRectSize();
    auto padding = ImGui::GetStyle().FramePadding;
    ImVec2 end_pos = ImVec2(begin_pos.x + item_size.x, begin_pos.y + item_size.y + padding.y * 2);
    ImRect bb(begin_pos, end_pos);
    ImGui::GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(color));
}
} // namespace Aurora