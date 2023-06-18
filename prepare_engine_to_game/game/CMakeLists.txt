cmake_minimum_required(VERSION 3.22)
project(game)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(game SHARED src/game.cxx)

if (APPLE)
    target_link_libraries(game PRIVATE engine_lib)
else ()
    target_link_libraries(game PRIVATE engine)
endif ()

add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:game>
        $<TARGET_FILE_DIR:engine>)
