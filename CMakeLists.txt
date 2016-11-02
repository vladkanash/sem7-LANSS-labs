cmake_minimum_required(VERSION 3.5)
project(LANSS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")

add_executable(server src/server/server.c src/main.c src/server/server.h src/constants.h src/types.h src/server/engine.h src/server/engine.c)
add_executable(client src/client.c src/types.h)


