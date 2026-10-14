cmake_minimum_required(VERSION 3.10)
project(Aurora)

option(SRC_DEBUG_MODE "Set src code to debug mode if ON" ON)
option(ENABLE_VCPKG OFF "Turn off vcpkg to ensure the compatibility of the third party libraries"
)
set(_shaderc_hint_paths "")
if (DEFINED ENV{VULKAN_SDK})
    list(APPEND _shaderc_hint_paths "$ENV{VULKAN_SDK}")
endif()
if (CMAKE_PREFIX_PATH)
    list(APPEND _shaderc_hint_paths ${CMAKE_PREFIX_PATH})
endif()

# vcpkg installs shaderc under the package name "unofficial-shaderc" and exports
# unofficial::shaderc::shaderc. Unlike a bare shaderc.lib, that target carries the
# static glslang/SPIRV-Tools dependencies, so prefer it whenever it is available.
find_package(unofficial-shaderc CONFIG QUIET)
if (TARGET unofficial::shaderc::shaderc AND NOT TARGET shaderc::shaderc)
    add_library(shaderc::shaderc INTERFACE IMPORTED)
    set_target_properties(shaderc::shaderc PROPERTIES
        INTERFACE_LINK_LIBRARIES unofficial::shaderc::shaderc
    )
endif()

find_package(shaderc CONFIG QUIET)
if (NOT shaderc_FOUND AND NOT TARGET shaderc::shaderc)
    # Fallback: locate Shaderc from Vulkan SDK without a CMake config package.
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

        # A plain shaderc.lib only contains libshaderc. When it is neither the
        # combined nor the shared build, the linker also needs libshaderc_util plus
        # the glslang and SPIRV-Tools static libraries sitting next to it. Without
        # them MSVC fails with LNK2019 (e.g. GlslangInitializer) in
        # shaderc_compiler_initialize.
        if (NOT SHADERC_LIB MATCHES "shaderc_combined" AND NOT SHADERC_LIB MATCHES "shaderc_shared")
            get_filename_component(_shaderc_lib_dir "${SHADERC_LIB}" DIRECTORY)
            set(_shaderc_dep_libs "")
            foreach(_shaderc_dep IN ITEMS
                    shaderc_util
                    glslang glslang-default-resource-limits
                    SPIRV MachineIndependent GenericCodeGen OGLCompiler OSDependent
                    HLSL SPVRemapper
                    SPIRV-Tools-opt SPIRV-Tools SPIRV-Tools-link)
                unset(_shaderc_dep_path CACHE)
                find_library(_shaderc_dep_path
                    NAMES ${_shaderc_dep}
                    HINTS "${_shaderc_lib_dir}"
                    NO_DEFAULT_PATH
                )
                if (_shaderc_dep_path)
                    list(APPEND _shaderc_dep_libs "${_shaderc_dep_path}")
                endif()
            endforeach()
            unset(_shaderc_dep_path CACHE)
            if (_shaderc_dep_libs)
                set_property(TARGET ${_shaderc_target} APPEND PROPERTY
                    INTERFACE_LINK_LIBRARIES "${_shaderc_dep_libs}"
                )
            endif()
        endif()

        # Provide non-namespaced aliases for older usage patterns.
        if (_shaderc_target STREQUAL "shaderc::shaderc_combined" AND NOT TARGET shaderc_combined)
            add_library(shaderc_combined ALIAS shaderc::shaderc_combined)
        endif()
        if (_shaderc_target STREQUAL "shaderc::shaderc" AND NOT TARGET shaderc)
            add_library(shaderc ALIAS shaderc::shaderc)
        endif()
    else()
        message(FATAL_ERROR "shaderc not found. Set shaderc_DIR, install the Vulkan SDK Shaderc component, or install vcpkg shaderc. Tried VULKAN_SDK='$ENV{VULKAN_SDK}'.")
    endif()
endif()

# If shaderc is found but no combined target is provided, try to locate it in prefix paths.
if (shaderc_FOUND AND NOT TARGET shaderc::shaderc_combined)
    find_library(SHADERC_COMBINED_LIB
        NAMES shaderc_combined
        HINTS ${_shaderc_hint_paths}
        PATH_SUFFIXES Lib lib
    )
    if (SHADERC_COMBINED_LIB)
        if (TARGET shaderc::shaderc)
            get_target_property(_shaderc_inc_dirs shaderc::shaderc INTERFACE_INCLUDE_DIRECTORIES)
        endif()
        add_library(shaderc::shaderc_combined UNKNOWN IMPORTED)
        set_target_properties(shaderc::shaderc_combined PROPERTIES
            IMPORTED_LOCATION "${SHADERC_COMBINED_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${_shaderc_inc_dirs}"
        )
        if (NOT TARGET shaderc_combined)
            add_library(shaderc_combined ALIAS shaderc::shaderc_combined)
        endif()
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
