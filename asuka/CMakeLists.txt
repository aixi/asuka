set(SOURCE_FILES
        client/BaseClient.cc)

add_library(asuka ${SOURCE_FILES})

target_link_libraries(asuka blaze luna)
install(TARGETS asuka DESTINATION lib)

set(HEADERS
        util.h
        Exception.h
        RpcError.h
        client/BaseClient.h)
install(FILES ${HEADERS} DESTINATION include)