I planned to support MSVC but it takes some pain to generate and load dll, which is far away from what I want to learn through this project, so no support for MSVC :( .

Please use MinGW if you want to have this project compiled. MSYS2 MINGW64 is recommended as the version of its g++ has a good support for C++17. g++ with versions lower than 9.0.0 may not work as std::filesystem is not supported in these versions.