cmake_minimum_required(VERSION 3.30)
project(webassembly_interpreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(webassembly_interpreter src/main.cpp src/Interpreter.cpp src/Parser.cpp src/TestRunner.cpp)

enable_testing()

add_library(InterpreterLib
        src/Parser.cpp
        src/Interpreter.cpp
)

target_include_directories(InterpreterLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src)
add_subdirectory(tests)