cmake_minimum_required(VERSION 3.14)
project(main CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-O3)

file( GLOB SOURCES "*.cpp")

# Define executable with your single source file
add_executable(main ${SOURCES})