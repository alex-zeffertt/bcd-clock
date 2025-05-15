cmake_minimum_required(VERSION 3.12)
project(example CXX)
add_executable(example main.cpp)
target_include_directories(example PRIVATE ../../../lib/fsm)

