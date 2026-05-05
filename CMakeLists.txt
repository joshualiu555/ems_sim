cmake_minimum_required(VERSION 4.3.2)

project(ems_sim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(main src/main.cpp)
