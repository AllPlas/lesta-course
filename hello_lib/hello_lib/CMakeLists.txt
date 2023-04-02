cmake_minimum_required(VERSION 3.18)

project(hello_lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(hello_lib_static STATIC include/hello.h src/hello.cpp)
target_include_directories(hello_lib_static PUBLIC include)

add_library(hello_lib_shared SHARED include/hello.h src/hello.cpp)
target_include_directories(hello_lib_shared PUBLIC include)

find_package(Catch2)
if (Catch2_FOUND)
    add_executable(hello_lib_tests tests/hello_lib_tests.cpp)
    target_link_libraries(hello_lib_tests PRIVATE Catch2::Catch2WithMain hello_lib_static)

    include(CTest)
    include(Catch)
    catch_discover_tests(hello_lib_tests)
endif ()