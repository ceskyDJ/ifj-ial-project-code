cmake_minimum_required(VERSION 3.20)
project(IFJ\ &\ IAL\ project\ \(code\) C)
set(CMAKE_SOURCE_DIR src/)
set(PROJECT_BINARY_DIR ../bin/)
set(CMAKE_BINARY_DIR ../bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -pedantic -Wall -Wextra -fsanitize=address")

add_executable(ifj21_compiler src/main.c)
