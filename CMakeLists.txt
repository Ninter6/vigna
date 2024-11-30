cmake_minimum_required(VERSION 3.29)
project(vigna)

set(CMAKE_CXX_STANDARD 20)

add_library(vigna INTERFACE)
target_include_directories(vigna INTERFACE src/)

add_subdirectory(sandbox)
