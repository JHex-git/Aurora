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

    SceneManager::GetInstance().LoadScene(FileSystem::GetFullPath("samples\\test.xml"));
    return true;
}

bool Engine::Tick()
{
    WindowSystem::GetInstance().PollEvents();
    RenderTick();
    return !WindowSystem::GetInstance().ShouldClose();
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
