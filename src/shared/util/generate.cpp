#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <string>

#include "models/call.hpp"


#include "generate.hpp"

Call generate_call(const Bounds &b, std::mt19937 &gen, int simulation_id) {
  std::uniform_int_distribution<int> priority(1, 15);
  std::uniform_int_distribution<int> x(b.x_min, b.x_max);
  std::uniform_int_distribution<int> y(b.y_min, b.y_max);

  Call c;
  c.simulation_id = simulation_id;
  c.id = 0;

  std::string d = "";

  int p = priority(gen);
  if (p == 1) {
    c.priority = CallPriority::Echo;
  } else if (p == 2 || p == 3) {
    c.priority = CallPriority::Delta;
  } else if (p >= 4 && p <= 6) {
    c.priority = CallPriority::Charlie;
  } else if (p >= 7 && p <= 10) {
    c.priority = CallPriority::Bravo;
  } else {
    c.priority = CallPriority::Alpha;
  }

  c.x = x(gen);
  c.y = y(gen);

  return c;
}