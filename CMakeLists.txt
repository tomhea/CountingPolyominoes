cmake_minimum_required(VERSION 3.15)
project(CountingPolyominoes)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-m64 -O3 -funroll-loops -Wall -Wextra")

add_library(RedelClient SHARED redelClient.cpp redelClient.h graphCreator.cpp graphCreator.h backups.cpp backups.h defs.h pybind11_wrapper.py.cpp)
add_library(RedelServer SHARED redelServer.cpp redelServer.h graphCreator.cpp graphCreator.h backups.cpp backups.h defs.h pybind11_wrapper.py.cpp)
