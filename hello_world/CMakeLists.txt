cmake_minimum_required(VERSION 3.22)
project(hello_world)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(hello_world src/main.cpp)

include(CTest)
add_test(NAME exit_with_zero COMMAND hello_world)
