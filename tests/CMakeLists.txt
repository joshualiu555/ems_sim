add_executable(tester
  test_util.cpp
  test_dispatch.cpp
  test_simulation.cpp
  test_database.cpp
  test_network.cpp
)

target_link_libraries(
  tester
  PRIVATE
  networking
  GTest::gtest_main
  asio_standalone
  nlohmann_json
)

include(GoogleTest)
gtest_discover_tests(tester)
