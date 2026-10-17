cmake_minimum_required(VERSION 3.20)
project(engine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Assimp
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.5
)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)

set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)

# FreeType
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HVF ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG VER-2-14-3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(freetype)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# OpenGL
find_package(OpenGL REQUIRED)

# shaderc
find_library(SHADERC_LIBRARY NAMES shaderc_combined shaderc REQUIRED)

# Engine
file(GLOB_RECURSE ENGINE_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)
add_library(engine STATIC 
    ${ENGINE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/src/glad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spirv_reflect/spirv_reflect.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb/stb_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb/stb_image_write.cpp
)

# make stuff available for users to include
target_include_directories(engine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spirv_reflect
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include
)

# declare dependencies that consumers need
target_link_libraries(engine 
    PUBLIC
        glm::glm
    PRIVATE 
        glfw 
        OpenGL::GL
        shaderc_combined
        glslang
        SPIRV-Tools-opt
        SPIRV-Tools
        assimp
        freetype
)

# engine resources
include(cmake/EmbedResources.cmake)
embed_resources(engine "${CMAKE_CURRENT_SOURCE_DIR}/res")