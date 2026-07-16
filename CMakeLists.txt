cmake_minimum_required(VERSION 4.2)
project(jiteval C)

set(CMAKE_C_STANDARD 11)

add_executable(jiteval benchmark.c)
