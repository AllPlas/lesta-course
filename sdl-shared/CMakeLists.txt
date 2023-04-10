cmake_minimum_required(VERSION 3.18)
project(sdl_shared)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED)

add_executable(sdl_shared src/main.cpp)
target_link_libraries(sdl_shared PRIVATE SDL3::SDL3-shared)

if (WIN32)
    add_custom_command(TARGET sdl_shared POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:sdl_shared>)
endif ()