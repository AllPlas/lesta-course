cmake_minimum_required(VERSION 3.18)

project(hello_bin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(hello_world_static src/main.cpp)
if (NOT APPLE)
    target_link_options(hello_world_static PRIVATE -static)
endif ()
target_link_libraries(hello_world_static PRIVATE hello_lib_static)

add_executable(hello_world_shared src/main.cpp)
target_link_libraries(hello_world_shared PRIVATE hello_lib_shared)
