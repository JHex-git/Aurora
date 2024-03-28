#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include

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
};
} // namespace Aurora