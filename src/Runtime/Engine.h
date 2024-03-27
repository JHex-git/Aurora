#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
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

    void Run();

private:
    std::unique_ptr<SceneManager> m_scene_manager;
};
} // namespace Aurora