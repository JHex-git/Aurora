// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Runtime/Engine.h"
#include "Core/render/WindowSystem.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/LightManager.h"
#include "Core/Render/RenderSystem.h"
#include "Runtime/Scene/SceneManager.h"

namespace Aurora
{

Engine::Engine()
{
}

Engine::~Engine()
{
}

bool Engine::Init()
{
    if (!WindowSystem::GetInstance().Init(1920, 1080, "Aurora"))
    {
        spdlog::error("Failed to initialize WindowSystem");
        return false;
    }
    spdlog::info("Window System Initialized");

    if (!RenderSystem::GetInstance().Init())
    {
        spdlog::error("Failed to initialize Editor UI");
        return false;
    }
    spdlog::info("Editor UI Initialized");

    return true;
}

bool Engine::Tick()
{
    WindowSystem::GetInstance().PollEvents();
    RenderTick();

    if (WindowSystem::GetInstance().ShouldClose())
    {
        return !RenderSystem::GetInstance().OnClose();
    }
    return true;
}

void Engine::RenderTick()
{
    WindowSystem::GetInstance().Clear();
    RenderSystem::GetInstance().Render();
    SceneManager::GetInstance().Update();
    WindowSystem::GetInstance().SwapBuffers();
}

void Engine::Run()
{
    while (Tick());
}
} // namespace Aurora
