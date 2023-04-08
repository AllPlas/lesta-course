cmake_minimum_required(VERSION 3.22)
project(sdl_shared)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED)

add_executable(sdl_shared src/main.cpp)
target_link_libraries(sdl_shared PRIVATE SDL3::SDL3-shared)
