cmake_minimum_required(VERSION 3.18)
project(sdl_engine_shared)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

find_package(SDL3 REQUIRED)

add_library(engine SHARED src/engine.hxx src/engine.cxx)
target_link_libraries(engine PRIVATE SDL3::SDL3-shared)

add_executable(game src/main.cxx)
target_link_libraries(game PRIVATE engine)

if (WIN32)
    add_custom_command(TARGET game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:game>

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:engine>
            $<TARGET_FILE_DIR:game>)
endif ()