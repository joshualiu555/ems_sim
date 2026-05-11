#include <gtest/gtest.h>

#include "models/helper.hpp"
#include "util/calc.hpp"

TEST(TimeTest, AddMinutesWithWrap) {
  Time t = {10, 50};

  Time next = find_next_time(t, 20);

  EXPECT_EQ(next.hour, 11);
  EXPECT_EQ(next.minute, 10);
}

TEST(TimeTest, AddMinutesWithoutWrap) {
  Time t = {10, 50};

  Time next = find_next_time(t, 5);

  EXPECT_EQ(next.hour, 10);
  EXPECT_EQ(next.minute, 55);
}

TEST(DistanceTest, CalculateDistance) {
  Location a = {0, 3}, b = {4, 0};
  double distance = find_distance(a, b);

  EXPECT_EQ(distance, 5.0);
}
