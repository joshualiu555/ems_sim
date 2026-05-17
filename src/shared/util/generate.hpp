#pragma once

#include <random>

#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/call.hpp"


struct Bounds {
  int x_min;
  int x_max;
  int y_min;
  int y_max;
};

Ambulance generate_ambulance(const Bounds &b, std::mt19937 &gen, int simulation_id);
Hospital generate_hospital(const Bounds &b, std::mt19937 &gen, int simulation_id);
Call generate_call(const Bounds &b, std::mt19937 &gen, int simulation_id);
