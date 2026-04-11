cmake_minimum_required(VERSION 3.10)

project(chapter8_8_22 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(mysystem 8.22.c)
add_executable(exit-code exit-code.c)
add_executable(wait-sig wait-sig.c)
