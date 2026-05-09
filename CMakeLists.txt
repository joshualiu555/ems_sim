cmake_minimum_required(VERSION 3.15)

project(ems_sim)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/d72f9c8aea6817cdf1ca0ac10887f328de7f3da2.zip
)

FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
  main
  src/main.cpp
  src/logic/dispatch.cpp
  src/logic/simulation.cpp
  src/logic/handle_event.cpp
  src/util/generate.cpp
  src/util/utility.cpp
  src/io/output.cpp
  src/io/logs.cpp
)

target_include_directories(main PRIVATE src)

enable_testing()

add_executable(
  tests
  tests/test_util.cpp
  tests/test_dispatch.cpp
  tests/test_simulation.cpp

  src/logic/dispatch.cpp
  src/logic/simulation.cpp
  src/logic/handle_event.cpp
  src/util/generate.cpp
  src/util/utility.cpp
  src/io/output.cpp
  src/io/logs.cpp
)

target_include_directories(tests PRIVATE src)

target_link_libraries(
  tests GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
