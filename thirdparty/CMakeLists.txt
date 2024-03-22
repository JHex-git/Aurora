add_compile_options("-DNDEBUG")
add_subdirectory(opengl/glm)
add_subdirectory(opengl/glfw)
add_subdirectory(opengl/glad)
add_subdirectory(spdlog)
add_subdirectory(assimp)

add_library(tinyxml2 STATIC tinyxml2/tinyxml2.cpp)
target_include_directories(tinyxml2 PUBLIC tinyxml2)