cmake_minimum_required(VERSION 3.18)
project(engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED)

add_executable(engine src/engine.cxx)

set_target_properties(
        engine PROPERTIES
        ENABLE_EXPORTS 1
)

target_include_directories(engine PUBLIC include)
target_link_libraries(engine PRIVATE SDL3::SDL3-shared)