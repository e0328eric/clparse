cmake_minimum_required(VERSION 3.21)

project(drapeau)

set(CMAKE_C_STANDARD 11)

add_executable(drapeau src/main.c)
target_compile_options(drapeau PRIVATE -ggdb -Wall -Wextra -Wpedantic)
target_include_directories(drapeau PRIVATE .)
