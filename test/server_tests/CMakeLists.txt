
add_executable(runServerTests
        server_test.c
        )

target_link_libraries(runServerTests cmocka)
target_link_libraries(runServerTests lanss)

add_test(NAME runServerTests COMMAND runServerTests)
