cmake_minimum_required(VERSION 3.12)
project(generate_dot)
add_executable(generate_dot  generate_dot.cpp)
target_include_directories(generate_dot PRIVATE ..)
