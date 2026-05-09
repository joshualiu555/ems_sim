cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

project(ems_sim)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/d72f9c8aea6817cdf1ca0ac10887f328de7f3da2.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
  libpqxx
  GIT_REPOSITORY https://github.com/jtv/libpqxx.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(googletest libpqxx)

add_library(engine STATIC
  src/logic/dispatch.cpp
  src/logic/simulation.cpp
  src/logic/handle_event.cpp
  src/util/generate.cpp
  src/util/utility.cpp
  src/io/output.cpp
  src/io/logs.cpp
)

target_include_directories(engine PUBLIC src)
target_link_libraries(engine PUBLIC pqxx)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE engine)

enable_testing()

add_executable(tests
  tests/test_util.cpp
  tests/test_dispatch.cpp
  tests/test_simulation.cpp
)

target_link_libraries(tests 
  PRIVATE
  engine
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
