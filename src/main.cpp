
#include <iostream>
#include "Runtime/Engine.h"

int main()
{


    auto engine = std::make_shared<Aurora::Engine>();
    
    if (engine->Init())
    {
        engine->Run();
    }
    return 0;
}