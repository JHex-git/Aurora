// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Runtime/Engine.h"
#include "core/render/WindowSystem.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/LightManager.h"


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
    m_window = std::make_unique<WindowSystem>();
    if (!m_window->Init(1920, 1080, "Aurora"))
    {
        spdlog::error("Failed to initialize WindowSystem");
        return false;
    }
    spdlog::info("Window System Initialized");

    m_scene_manager = std::make_unique<SceneManager>();
    m_scene_manager->LoadScene(FileSystem::GetFullPath("samples\\test.xml"));
    LightManager::GetInstance().AddLight(Light{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)});
    return true;
}

bool Engine::Tick()
{
    m_window->PollEvents();
    RenderTick();
    return !m_window->ShouldClose();
}

void Engine::RenderTick()
{
    m_window->Clear();
    m_scene_manager->Update();
    m_window->SwapBuffers();
}
} // namespace Aurora
