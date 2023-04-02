cmake_minimum_required(VERSION 3.18)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

project(hello_world_2)

add_subdirectory(hello_lib)
add_subdirectory(hello_bin)
