cmake_minimum_required(VERSION 3.28)
project(calc C)

set(CMAKE_C_STANDARD 23)

add_executable(calc main.c)

project(tests C)

set(CMAKE_C_STANDARD 23)
add_executable(tests tests.c)
