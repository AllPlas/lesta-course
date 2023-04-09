cmake_minimum_required(VERSION 3.22)
project(sdl_static)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED)

add_executable(sdl_static src/main.cpp)
if (NOT APPLE)
    target_link_options(sdl_static PRIVATE -static)
endif ()
target_link_libraries(sdl_static PRIVATE SDL3::SDL3-static)