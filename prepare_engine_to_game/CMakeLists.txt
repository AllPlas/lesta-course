cmake_minimum_required(VERSION 3.18)

if (EXISTS ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif ()

project(engine_prepare_to_game)

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

add_subdirectory(engine)
add_subdirectory(game)