cmake_minimum_required(VERSION 3.22)
project(hello_lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

add_library(hello_lib_static STATIC src/hello.h src/hello.cpp)
target_include_directories(hello_lib_static PUBLIC src)

add_library(hello_lib_shared SHARED src/hello.h src/hello.cpp)
target_include_directories(hello_lib_shared PUBLIC src)

add_executable(hello_lib_tests tests/hello_lib_tests.cpp)
target_link_libraries(hello_lib_tests PRIVATE CONAN_PKG::catch2 hello_lib_static)

include(CTest)
include(${CONAN_BUILD_DIRS_CATCH2_DEBUG}/Catch.cmake)
catch_discover_tests(hello_lib_tests)