cmake_minimum_required(VERSION 3.20)
project(example LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(example
    src/main.cpp
)

# add engine as a subproject
add_subdirectory(
    ../engine
    ${CMAKE_CURRENT_BINARY_DIR}/engine
)

# link against engine
target_link_libraries(example
    PRIVATE
        engine
)