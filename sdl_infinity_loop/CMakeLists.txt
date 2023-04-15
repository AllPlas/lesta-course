cmake_minimum_required(VERSION 3.18)
project(sdl_infinity_loop)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED)

add_executable(sdl_infinity_loop src/main.cxx)

target_link_libraries(sdl_infinity_loop PRIVATE SDL3::SDL3-shared)
