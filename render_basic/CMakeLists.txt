cmake_minimum_required(VERSION 3.25)

if (EXISTS ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif ()

project(render_basic)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 REQUIRED)
find_package(SDL3 REQUIRED)

add_executable(render_basic
        src/main.cxx
        src/canvas.hxx
        src/line_render.hxx
        src/triangle_render.hxx
        src/triangle_indexed_render.hxx
        src/triangle_interpolated.hxx)

add_executable(sdl_render src/sdl_main.cxx src/gfx_program.hxx)

add_executable(render_basic_tests tests/canvas_tests.cxx tests/draw_line_tests.cxx)
target_link_libraries(render_basic_tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(sdl_render PRIVATE SDL3::SDL3-static)