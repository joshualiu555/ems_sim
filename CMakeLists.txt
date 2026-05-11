cmake_minimum_required(VERSION 3.15)

project(ems_sim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(dep)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
