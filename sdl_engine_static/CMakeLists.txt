cmake_minimum_required(VERSION 3.25)
project(sdl_engine_static)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED)

add_library(engine STATIC src/engine.hxx src/engine.cxx)
target_link_libraries(engine PRIVATE SDL3::SDL3-shared)

add_executable(game_with_engine src/main.cxx)
target_link_libraries(game_with_engine PRIVATE engine)