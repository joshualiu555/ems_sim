#pragma once

#include <random>

#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/call.hpp"
#include "models/helper.hpp"

struct Bounds {
  int lat_west;
  int lat_east;
  int lon_north;
  int lon_south;
};

Ambulance generate_ambulance(const Bounds &b, std::mt19937 &gen, int simulation_id);
Hospital generate_hospital(const Bounds &b, std::mt19937 &gen, int simulation_id);
Call generate_call(const Bounds &b, std::mt19937 &gen, int simulation_id);
