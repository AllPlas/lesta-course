cmake_minimum_required(VERSION 3.18)
project(engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(engine src/engine.cxx glad/src/glad.c)

set_target_properties(
        engine PROPERTIES
        ENABLE_EXPORTS 1
)

target_include_directories(engine PUBLIC include)
target_include_directories(engine PRIVATE glad/include)
target_link_libraries(engine PRIVATE SDL3::SDL3-shared OpenGL::GL)

if (WIN32)
    add_custom_command(TARGET engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:engine>)
endif ()