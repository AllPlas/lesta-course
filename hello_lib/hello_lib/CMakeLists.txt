cmake_minimum_required(VERSION 3.22)
project(hello_lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(hello_lib_static STATIC src/hello.h src/hello.cpp)
target_include_directories(hello_lib_static PUBLIC src)

add_library(hello_lib_shared SHARED src/hello.h src/hello.cpp)
target_include_directories(hello_lib_shared PUBLIC src)