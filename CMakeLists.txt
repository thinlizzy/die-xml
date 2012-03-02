# CMake build files for the die-xml project.

cmake_minimum_required (VERSION 2.8)
project ("die-xml" CXX)

if (UNIX OR CYGWIN)
    add_definitions(-std=c++0x -Wall)
endif()

add_subdirectory(src)
add_subdirectory(test)

