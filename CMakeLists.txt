cmake_minimum_required(VERSION 3.10)
project(Aurora)

option(SRC_DEBUG_MODE "Set src code to debug mode if ON" ON)
option(ENABLE_VCPKG OFF "Turn off vcpkg to ensure the compatibility of the third party libraries"
)
find_package(shaderc CONFIG QUIET)
if (NOT shaderc_FOUND)
    # Fallback: locate Shaderc from Vulkan SDK without a CMake config package.
    set(_shaderc_hint_paths "")
    if (DEFINED ENV{VULKAN_SDK})
        list(APPEND _shaderc_hint_paths "$ENV{VULKAN_SDK}")
    endif()

    find_path(SHADERC_INCLUDE_DIR
        NAMES shaderc/shaderc.hpp
        HINTS ${_shaderc_hint_paths}
        PATH_SUFFIXES Include include
    )

    find_library(SHADERC_LIB
        NAMES shaderc_combined shaderc_shared shaderc
        HINTS ${_shaderc_hint_paths}
        PATH_SUFFIXES Lib lib
    )

    if (SHADERC_INCLUDE_DIR AND SHADERC_LIB)
        if (SHADERC_LIB MATCHES "shaderc_combined")
            set(_shaderc_target "shaderc::shaderc_combined")
        else()
            set(_shaderc_target "shaderc::shaderc")
        endif()

        add_library(${_shaderc_target} UNKNOWN IMPORTED)
        set_target_properties(${_shaderc_target} PROPERTIES
            IMPORTED_LOCATION "${SHADERC_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${SHADERC_INCLUDE_DIR}"
        )

        # Provide non-namespaced aliases for older usage patterns.
        if (_shaderc_target STREQUAL "shaderc::shaderc_combined" AND NOT TARGET shaderc_combined)
            add_library(shaderc_combined ALIAS shaderc::shaderc_combined)
        endif()
        if (_shaderc_target STREQUAL "shaderc::shaderc" AND NOT TARGET shaderc)
            add_library(shaderc ALIAS shaderc::shaderc)
        endif()
    else()
        message(FATAL_ERROR "shaderc not found. Set shaderc_DIR or install Vulkan SDK Shaderc component. Tried VULKAN_SDK='$ENV{VULKAN_SDK}'.")
    endif()
endif()
if (SRC_DEBUG_MODE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SKIP_INSTALL_RULES YES)

if(MSVC)  
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    add_compile_options(/wd4819 /wd4267 /wd4244 /wd4996)  
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
include(${CMAKE_SOURCE_DIR}/cmake/glfw.cmake)
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

if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Engine")
    target_link_libraries(${PROJECT_NAME} PUBLIC Editor_lib)
endif()
