#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/WindowSystem.h"
#include "Runtime/Scene/SceneManager.h"

namespace Aurora
{

class Engine
{
public:
    Engine();
    ~Engine();

    bool Init();
    bool Tick();

    void RenderTick();

private:
    std::unique_ptr<WindowSystem> m_window;
    std::unique_ptr<SceneManager> m_scene_manager;
};
} // namespace Aurora