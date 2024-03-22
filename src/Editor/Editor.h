#pragma once
#include <memory>

namespace Aurora
{
class Engine;

class Editor
{
public:
    Editor(std::shared_ptr<Engine> engine);
    void Run();

private:
    std::shared_ptr<Engine> m_Engine;
};
} // namespace Aurora
