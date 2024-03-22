file(GLOB IMGUI_SRCS ${CMAKE_SOURCE_DIR}/thirdparty/imgui/*.cpp)
list(APPEND IMGUI_SRCS ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_glfw.cpp ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_opengl3.cpp)
add_library(imgui STATIC ${IMGUI_SRCS})
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/imgui/)