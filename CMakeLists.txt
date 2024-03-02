cmake_minimum_required(VERSION 2.9)
project(prudynt)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 20)

aux_source_directory(./src SRC_LIST)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Og -g -DNO_OPENSSL=1")
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(./src)
