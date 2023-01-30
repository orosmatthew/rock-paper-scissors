cmake_minimum_required(VERSION 3.20)

project(rock_paper_scissors)

set(CMAKE_CXX_STANDARD 20)

if (EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fwasm-exceptions --preload-file res -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s EXPORTED_FUNCTIONS=\"['_main', '_malloc']\" -s \"EXPORTED_RUNTIME_METHODS=['ccall']\"")
endif ()

add_subdirectory(lib/raylib-4.2.0)
add_subdirectory(lib/raylib-cpp-4.2.7)

set(LIB_INCLUDES
        lib/raygui-3.2/include
        )

set(SOURCE_FILES
        src/main.cpp
        src/fixed_loop.cpp
        src/rock_paper_scissors.cpp
        )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${LIB_INCLUDES})

target_link_libraries(${PROJECT_NAME} raylib raylib_cpp)