cmake_minimum_required(VERSION 3.18)
project(hello_lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
    conan_basic_setup(TARGETS)
endif ()

add_library(hello_lib_static STATIC src/hello.h src/hello.cpp)
target_include_directories(hello_lib_static PUBLIC src)

add_library(hello_lib_shared SHARED src/hello.h src/hello.cpp)
target_include_directories(hello_lib_shared PUBLIC src)

add_executable(hello_lib_tests tests/hello_lib_tests.cpp)
target_link_libraries(hello_lib_tests PRIVATE CONAN_PKG::catch2 hello_lib_static)

if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
    include(CTest)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        include(${CONAN_BUILD_DIRS_CATCH2_DEBUG}/Catch.cmake)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        include(${CONAN_BUILD_DIRS_CATCH2_RELEASE}/Catch.cmake)
    endif ()

    catch_discover_tests(hello_lib_tests)
endif ()