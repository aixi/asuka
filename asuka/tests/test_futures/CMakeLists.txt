include_directories(${PROJECT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)

add_executable(future_test TestFuture.cc)

add_executable(tryvoid_test TestTryVoid.cc)