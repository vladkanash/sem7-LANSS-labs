cmake_minimum_required(VERSION 3.5)
project(LANSS)

enable_testing()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")

include_directories(src)

add_subdirectory(test)
add_subdirectory(src)

add_executable(server main.c)
target_link_libraries(server lanss)


