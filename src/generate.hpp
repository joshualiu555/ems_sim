#include <random>

#include "models.hpp"

struct Bounds {
  int lat_west;
  int lat_east;
  int lon_north;
  int lon_south;
};

int generate_id();

Ambulance generate_ambulance(Bounds b, std::mt19937 &gen);

Hospital generate_hospital(Bounds b, std::mt19937 &gen);

Call generate_call(Bounds b, std::mt19937 &gen);
