CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

include_directories(${PROJECT_SOURCE_DIR})

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_library(coroutine Coroutine.cc)
