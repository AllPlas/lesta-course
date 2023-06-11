cmake_minimum_required(VERSION 3.18)
project(game)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(game SHARED src/game.cxx)
target_link_libraries(game PRIVATE engine)


add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:game>
        $<TARGET_FILE_DIR:engine>)
