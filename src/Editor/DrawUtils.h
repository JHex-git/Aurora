#pragma once
// std include
#include <string>
#include "thirdparty/imgui/imgui.h"
// thirdparty include

// Aurora include


namespace Aurora
{

class DrawUtils
{
public:
    static void DrawTextBackground(const std::string& text, const ImVec4& color);
    static void DrawRectNoPadding(const ImVec4& color);
};
} // namespace Aurora