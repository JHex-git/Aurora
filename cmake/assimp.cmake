include_directories(${CMAKE_SOURCE_DIR}/thirdparty/assimp/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/thirdparty/assimp/include)  
set(ASSIMP_INSTALL OFF CACHE BOOL "Disable this if you want to use assimp as a submodule." FORCE)  
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable Assimp unit test build." FORCE)
if(NOT MSVC) # MSVC debug build fail with shared assimp due to fatal error LNK1189
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build assimp as a shared library." FORCE)
elseif(MINGW)
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable this if you want to use assimp as a submodule." FORCE)
endif()