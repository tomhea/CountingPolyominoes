cmake_minimum_required(VERSION 3.15)
project(CountingPolyominoes)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3 -funroll-loops -Wall -Wextra")

add_executable(CountingPolyominoes redelServer.cpp redelClient.cpp graphCreator.cpp graphCreator.h main.cpp redelClient.h redelServer.h main.cpp backups.cpp backups.h defs.h)