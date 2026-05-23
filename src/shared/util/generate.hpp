#pragma once

#include <random>

#include "models/call.hpp"


struct Bounds {
  int x_min;
  int x_max;
  int y_min;
  int y_max;
};

Call generate_call(const Bounds &b, std::mt19937 &gen, int simulation_id);
