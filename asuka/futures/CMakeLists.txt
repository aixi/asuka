cmake_minimum_required(VERSION 3.0)

include_directories(${PROJECT_SOURCE_DIR})

set(HEADERS
        Future.h
        Try.h
        Helper.h)

install(FILES ${HEADERS} DESTINATION include/asuka/future)