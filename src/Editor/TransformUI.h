#pragma once
// std include

// thirdparty include

// Aurora include
#include "Editor/ComponentUI.h"

namespace Aurora
{


class TransformUI : public ComponentUI
{
public:
    ComponentLayoutFunction Layout() override;
};
} // namespace Aurora