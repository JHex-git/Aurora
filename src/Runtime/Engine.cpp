// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Runtime/Engine.h"
#include "Core/render/WindowSystem.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/LightManager.h"
#include "Core/Render/RenderSystem.h"

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


    m_scene_manager = std::make_unique<SceneManager>();
    m_scene_manager->LoadScene(FileSystem::GetFullPath("samples\\test.xml"));
    LightManager::GetInstance().AddLight(Light{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)});
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
    m_scene_manager->Update();
    WindowSystem::GetInstance().SwapBuffers();
}

void Engine::Run()
{
    while (Tick());
}
} // namespace Aurora
