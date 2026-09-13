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

# copy res next to the executable
add_custom_command(TARGET example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:example>/res
)