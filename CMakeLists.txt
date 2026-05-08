cmake_minimum_required(VERSION 4.3.2)

project(ems_sim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
  main
  src/main.cpp
  src/logic/dispatch.cpp
  src/logic/simulation.cpp
  src/util/generate.cpp
  src/util/utility.cpp
  src/io/output.cpp
  src/io/logs.cpp
)
