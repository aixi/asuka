cmake_minimum_required(VERSION 3.0)

project(asuka)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CXX_FLAGS
        -Wall
        -Wextra
        -Werror
        -Wconversion
        -Wno-unused-parameter
        -Wold-style-cast
        -Wsequence-point
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wwrite-strings
        -march=native
        -std=c++17
        -Wno-null-dereference
        -Wno-sign-conversion
        -Wno-unused-private-field
        -Wthread-safety
        -fmodules)

if(CMAKE_BUILD_BITS EQUAL 32)
    list(APPEND CXX_FLAGS "-m32")
endif()

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/third_party")

link_directories("${PROJECT_SOURCE_DIR}/third_party")

link_libraries(pthread)

add_subdirectory(asuka)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
    add_subdirectory(examples)
endif()