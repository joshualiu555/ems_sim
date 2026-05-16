#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <string>

#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/call.hpp"
#include "models/helper.hpp"

#include "generate.hpp"
#include "calc.hpp"

Ambulance generate_ambulance(const Bounds &b, std::mt19937 &gen, int simulation_id) {
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
    0, // dummy id that will later be replaced by the database, the source of truth
    simulation_id,
    as,
    at,
    l,
    l
  };

  return a;
}

Hospital generate_hospital(const Bounds &b, std::mt19937 &gen, int simulation_id) {
  std::uniform_real_distribution<double> lat(b.lat_west, b.lat_east);
  std::uniform_real_distribution<double> lon(b.lon_north, b.lon_south);
  std::uniform_int_distribution<int> capacity(1, 5);

  int c = capacity(gen);
  Location l = {
    lat(gen),
    lon(gen)
  };

  Hospital h = {
    0,
    simulation_id,
    0,
    c,
    l
  };

  return h;
}

Call generate_call(const Bounds &b, std::mt19937 &gen, int simulation_id) {
  std::uniform_int_distribution<int> priority(1, 5);
  std::uniform_real_distribution<double> lat(b.lat_west, b.lat_east);
  std::uniform_real_distribution<double> lon(b.lon_north, b.lon_south);

  Call c;
  c.simulation_id = simulation_id;
  c.id = 0;

  std::string d = "";

  int p = priority(gen);
  switch(p) {
    case 1:
      c.priority = CallPriority::Echo;
      break;
    case 2:
      c.priority = CallPriority::Delta;
      break;
    case 3:
      c.priority = CallPriority::Charlie;
      break;
    case 4:
      c.priority = CallPriority::Bravo;
      break;
    case 5:
      c.priority = CallPriority::Alpha;
      break;
    default:
        break;
  }

  Location l = {
    lat(gen),
    lon(gen)
  };

  c.location = l;

  return c;
}
