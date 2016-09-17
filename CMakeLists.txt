cmake_minimum_required(VERSION 3.5)
project(LANSS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(server src/server.c src/commands.h)
add_executable(client src/client.c)


