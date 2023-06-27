cmake_minimum_required(VERSION 3.22)
project(game)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(game SHARED
        src/game.cxx
        src/ship.cxx
        src/ship.hxx
        src/island.cxx
        src/island.hxx
        src/map.cxx
        src/map.hxx
        src/player.cxx
        src/player.hxx
        src/bottle.cxx
        src/bottle.hxx
        src/treasure.cxx
        src/treasure.hxx
        src/menu.cxx
        src/menu.hxx
        src/config.hxx)

if (APPLE)
    target_link_libraries(game PRIVATE engine_lib)
else ()
    target_link_libraries(game PRIVATE engine)
endif ()

add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:game>
        $<TARGET_FILE_DIR:engine>)