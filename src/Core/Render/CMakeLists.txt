file(GLOB_RECURSE SRCS *.cpp)
set(TARGET Render_lib)
add_library(${TARGET} STATIC ${SRCS})
target_link_libraries(${TARGET} PUBLIC glad glWrapper_lib glfw glWrapper_lib Scene_lib imgui)