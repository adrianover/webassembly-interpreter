cmake_minimum_required(VERSION 3.30)
project(webassembly_interpreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(webassembly_interpreter src/main.cpp src/Parser.cpp)
