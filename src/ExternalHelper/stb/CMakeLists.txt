add_compile_options("-DNDEBUG")
file(GLOB SRCS *.cpp)
add_library(stb SHARED ${SRCS})