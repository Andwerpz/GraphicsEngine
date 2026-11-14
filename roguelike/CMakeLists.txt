cmake_minimum_required(VERSION 3.20)
project(roguelike LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE ROGUELIKE_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)
add_executable(roguelike ${ROGUELIKE_SOURCES})
target_include_directories(roguelike
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# add engine as a subproject
add_subdirectory(../engine engine)

# link against engine
target_link_libraries(roguelike
    PRIVATE
        engine
)

# copy res whenever any resource file changes
file(GLOB_RECURSE RES_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/res/*
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/res.stamp
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:roguelike>/res
    COMMAND ${CMAKE_COMMAND} -E touch
        ${CMAKE_CURRENT_BINARY_DIR}/res.stamp
    DEPENDS ${RES_FILES}
)

add_custom_target(copy_res ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/res.stamp
)

add_dependencies(roguelike copy_res)