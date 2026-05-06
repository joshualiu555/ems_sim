#include <random>
#include <utility>

#include "generate.hpp"
#include "models.hpp"

int generate_id() {
  static int id = 0;
  return id++;
}

Ambulance generate_ambulance(Bounds b, std::mt19937 &gen) {
  int id = generate_id();
  std::uniform_real_distribution<double> lat(b.lat_west, b.lat_east);
  std::uniform_real_distribution<double> lon(b.lon_north, b.lon_south);
  std::bernoulli_distribution als(0.5);

  AmbulanceStatus as = AmbulanceStatus::Available;
  AmbulanceType at = als(gen) ? AmbulanceType::ALS : AmbulanceType::BLS;
  Location l = {
    lat(gen),
    lon(gen)
  };

  Ambulance a = {
    id,
    as,
    at,
    l
  };

  return a;
}