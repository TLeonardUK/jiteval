cmake_minimum_required(VERSION 4.2)
project(jiteval C)

set(CMAKE_C_STANDARD 11)

set(JITEVAL_ISA "x86" CACHE STRING "")

if (JITEVAL_ISA STREQUAL "x86")
    message( INFO "A")
    if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        # Defined with CMAKE_GENERATOR_PLATFORM
    elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_C_FLAGS -m32)
        set(CMAKE_CXX_FLAGS -m32)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_C_FLAGS -m32)
        set(CMAKE_CXX_FLAGS -m32)
    endif()
elseif (JITEVAL_ISA STREQUAL "x64")
    if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        # Defined with CMAKE_GENERATOR_PLATFORM
    elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        # Default
    elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        # Default
    endif()
else()
    message( FATAL_ERROR "Specified JITEVAL_ISA is unimplemented" )
endif()

add_executable(jiteval benchmark.c)
