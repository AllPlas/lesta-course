cmake_minimum_required(VERSION 3.18)
project(sdl_engine_exe)

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

add_subdirectory(engine)
add_subdirectory(game)