include_directories(${PROJECT_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/tests/coroutine_test)

add_executable(coroutine_test TestCoroutine.cc)

target_link_libraries(coroutine_test coroutine)

add_dependencies(coroutine_test coroutine)