cmake_minimum_required(VERSION 3.28)
project(calc C)

set(CMAKE_C_STANDARD 23)

if (${CMAKE_BUILD_TYPE} MATCHES “Debug”)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -fsanitize=leak -fno-omit-frame-pointer")
endif ()

link_libraries(m)
# add_executable(null parser.c)
add_executable(calc main.c)