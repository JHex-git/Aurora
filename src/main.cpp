
#include <iostream>
#include "Runtime/Engine.h"
#include "Editor/Editor.h"

int main()
{


    auto engine = std::make_shared<Aurora::Engine>();
    
    if (engine->Init())
    {
        Aurora::Editor* editor = new Aurora::Editor(engine);
        editor->Run();

        delete editor;
    }
    return 0;
}