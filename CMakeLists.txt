cmake_minimum_required(VERSION 3.15)

project(ems_sim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/d72f9c8aea6817cdf1ca0ac10887f328de7f3da2.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
  libpqxx
  URL https://github.com/jtv/libpqxx/archive/8.0.1.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googletest libpqxx)

add_library(engine STATIC
  src/logic/dispatch.cpp
  src/logic/handle_event.cpp
  src/logic/simulation.cpp
  src/util/convert.cpp
  src/util/generate.cpp
  src/util/utility.cpp
  src/io/logs.cpp
  src/io/output.cpp
  src/config/config.cpp
  src/db/postgres.cpp
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

target_link_libraries(
  tests
  PRIVATE
  engine
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
