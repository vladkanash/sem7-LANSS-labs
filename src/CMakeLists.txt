project(LANSS)

set(HEADER_FILES
        server/server.h
        server/engine.h
        system_dependent_code.h
        constants.h
        types.h
        )

set(SOURCE_FILES
        server/server.c
        server/engine.c
        system_dependent_code.c
        )

add_library(lanss STATIC ${SOURCE_FILES} ${HEADER_FILES})

add_executable(server server/main.c)
add_executable(client client/client.h client/client.c)

target_link_libraries(server lanss)
target_link_libraries(client lanss)