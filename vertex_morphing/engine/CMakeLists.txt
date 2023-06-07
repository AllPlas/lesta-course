cmake_minimum_required(VERSION 3.18)
project(engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED)
find_package(PNG REQUIRED)
find_package(glm REQUIRED)
find_package(ImGui REQUIRED)

add_executable(engine
        src/engine.cxx
        glad/src/glad.c
        src/hot_reload_provider.hxx
        src/hot_reload_provider.cxx
        src/program.cxx
        src/program.hxx
        src/opengl_check.cxx
        src/opengl_check.hxx
        src/texture.cxx
        src/texture.hxx
        src/imgui_impl_sdl3.cxx
        src/imgui_impl_sdl3.hxx
        src/buffer.cxx
        src/buffer.hxx)

set_target_properties(
        engine PROPERTIES
        ENABLE_EXPORTS 1
)

target_include_directories(engine PUBLIC include)
target_include_directories(engine PRIVATE glad/include)
target_link_libraries(engine PRIVATE SDL3::SDL3-shared OpenGL::GL PNG::PNG boost::boost glm::glm imgui::imgui)

if (WIN32)
    add_custom_command(TARGET engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:engine>)
endif ()