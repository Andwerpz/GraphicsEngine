cmake_minimum_required(VERSION 3.20)
project(example LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(example
    src/main.cpp
)

# add engine as a subproject
add_subdirectory(../engine engine)

# link against engine
target_link_libraries(example
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
        $<TARGET_FILE_DIR:example>/res
    COMMAND ${CMAKE_COMMAND} -E touch
        ${CMAKE_CURRENT_BINARY_DIR}/res.stamp
    DEPENDS ${RES_FILES}
)

add_custom_target(copy_res ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/res.stamp
)

add_dependencies(example copy_res)