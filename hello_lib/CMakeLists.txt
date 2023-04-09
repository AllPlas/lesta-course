cmake_minimum_required(VERSION 3.18)

if (EXISTS ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif ()

project(hello_world_2)

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

add_subdirectory(hello_lib)
add_subdirectory(hello_bin)
