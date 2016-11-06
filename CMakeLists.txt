cmake_minimum_required(VERSION 3.5)
project(LANSS)

enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(src)

add_subdirectory(test)
add_subdirectory(src)


