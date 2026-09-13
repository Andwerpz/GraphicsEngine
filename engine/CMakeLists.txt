cmake_minimum_required(VERSION 3.20)
project(engine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
)

# make stuff available for users to include
target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include
)

# declare dependencies that consumers need
target_link_libraries(engine 
    PRIVATE 
        glfw 
        OpenGL::GL
        ${SHADERC_LIBRARY}
)
