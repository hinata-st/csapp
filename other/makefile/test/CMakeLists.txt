cmake_minimum_required(VERSION 3.10)

project(HelloWorld)

set(SOURCES main.c message.c)

add_executable(hello ${SOURCES})
