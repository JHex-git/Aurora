cmake_minimum_required(VERSION 3.10)
project(Aurora)

option(SRC_DEBUG_MODE "Set src code to debug mode if ON" ON)
option(ENABLE_VCPKG OFF "Turn off vcpkg to ensure the compatibility of the third party libraries"
)
if (SRC_DEBUG_MODE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)  
    add_compile_options(/wd4819)  
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

    if (${CMAKE_BUILD_TYPE} STREQUAL "DEBUG")
        set(BUILD_SUB_DIR "Debug")
    else()
        set(BUILD_SUB_DIR "Release")
    endif()
endif()  

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  

include(${CMAKE_SOURCE_DIR}/cmake/spdlog.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/assimp.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/glad.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/imgui.cmake)
add_subdirectory(thirdparty)

add_definitions(-DPROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")

add_subdirectory(src/ExternalHelper)
add_subdirectory(src/Math)
add_subdirectory(src/Core)
add_subdirectory(src/Editor)
add_subdirectory(src/Runtime)
add_subdirectory(src/glWrapper)
add_executable(${PROJECT_NAME} src/main.cpp)

if (MINGW)
    target_link_libraries(${PROJECT_NAME} PUBLIC Editor_lib)
elseif(MSVC)
    target_link_libraries(${PROJECT_NAME} PUBLIC Editor_lib)
endif()
