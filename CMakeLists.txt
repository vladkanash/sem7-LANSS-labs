cmake_minimum_required(VERSION 3.5)
project(LANSS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")

add_executable(server src/server.c src/main.c src/server.h src/commands.h)
add_executable(client src/client.c)


