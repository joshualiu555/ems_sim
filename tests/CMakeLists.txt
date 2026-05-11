add_executable(tester
  test_util.cpp
  test_dispatch.cpp
  test_simulation.cpp
)

target_link_libraries(
  tester
  PRIVATE
  engine
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tester)
