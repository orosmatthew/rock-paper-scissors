cmake_minimum_required(VERSION 3.20)

project(rock_paper_scissors)

set(CMAKE_CXX_STANDARD 20)

# set(PACKAGE_OPTION OFF CACHE BOOL "")

add_subdirectory(lib/raylib-4.2.0)
add_subdirectory(lib/raylib-cpp-4.2.7)

set(SOURCE_FILES
        src/main.cpp
        src/fixed_loop.cpp
        src/rock_paper_scissors.cpp
        )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} raylib raylib_cpp)