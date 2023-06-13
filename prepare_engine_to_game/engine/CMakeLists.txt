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

if (APPLE)
    add_library(engine_lib SHARED
            src/engine.cxx
            glad/src/glad.c
            src/hot_reload_provider.hxx
            src/hot_reload_provider.cxx
            src/program.cxx
            include/program.hxx
            src/opengl_check.cxx
            src/opengl_check.hxx
            src/texture.cxx
            include/texture.hxx
            src/imgui_impl_sdl3.cxx
            src/imgui_impl_sdl3.hxx
            src/buffer.cxx
            include/buffer.hxx
            src/imgui_impl_opengl3.cxx
            src/imgui_impl_opengl3.hxx
            src/sprite.cxx
            include/sprite.hxx)

    target_include_directories(engine_lib PUBLIC include)
    target_include_directories(engine_lib PRIVATE glad/include)
    target_link_libraries(engine_lib PRIVATE SDL3::SDL3-shared OpenGL::GL PNG::PNG boost::boost)
    target_link_libraries(engine_lib PUBLIC glm::glm imgui::imgui)

    add_executable(engine dd.cxx)
    target_link_libraries(engine PRIVATE engine_lib)
else ()
    add_executable(engine
            src/engine.cxx
            glad/src/glad.c
            src/hot_reload_provider.hxx
            src/hot_reload_provider.cxx
            src/program.cxx
            include/program.hxx
            src/opengl_check.cxx
            src/opengl_check.hxx
            src/texture.cxx
            include/texture.hxx
            src/imgui_impl_sdl3.cxx
            src/imgui_impl_sdl3.hxx
            src/buffer.cxx
            include/buffer.hxx
            src/imgui_impl_opengl3.cxx
            src/imgui_impl_opengl3.hxx
            src/sprite.cxx
            include/sprite.hxx)

    set_target_properties(engine PROPERTIES ENABLE_EXPORTS 1)

    target_include_directories(engine PUBLIC include)
    target_include_directories(engine PRIVATE glad/include)
    target_link_libraries(engine PRIVATE SDL3::SDL3-shared OpenGL::GL PNG::PNG boost::boost)
    target_link_libraries(engine PUBLIC glm::glm imgui::imgui)
endif ()

if (WIN32)
    add_custom_command(TARGET engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:engine>)
endif ()