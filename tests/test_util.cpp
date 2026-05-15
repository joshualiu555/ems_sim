#include <gtest/gtest.h>

#include "models/helper.hpp"
#include "util/calc.hpp"

TEST(DistanceTest, CalculateDistance) {
  Location a = {0, 3}, b = {4, 0};
  double distance = find_distance(a, b);

  EXPECT_EQ(distance, 5.0);
}
