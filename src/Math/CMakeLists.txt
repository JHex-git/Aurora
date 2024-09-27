file(GLOB SRCS *.cpp)
set(TARGET Math_lib)
add_library(${TARGET} STATIC ${SRCS})
target_link_libraries(${TARGET} PUBLIC glm)