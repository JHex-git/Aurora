#include <iostream>

#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"

#include "Editor/Editor.h"
#include "Runtime/Engine.h"

namespace Aurora
{
Editor::Editor(std::shared_ptr<Engine> engine)
    : m_Engine(engine) {}

void Editor::Run()
{
    while (true)
    {
        if (!m_Engine->Tick()) break;
    }
    
}
}