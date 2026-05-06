#include <iostream>
#include <random>
#include <string>
#include <utility>

#include "models.hpp"
#include "generate.hpp"

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

Hospital generate_hospital(Bounds b, std::mt19937 &gen) {
  int id = generate_id();
  std::uniform_real_distribution<double> lat(b.lat_west, b.lat_east);
  std::uniform_real_distribution<double> lon(b.lon_north, b.lon_south);
  std::uniform_int_distribution<int> capacity(100, 200);

  int c = capacity(gen);
  Location l = {
    lat(gen),
    lon(gen)
  };

  Hospital h = {
    id,
    c,
    l
  };

  return h;
}

Call generate_call(Bounds b, std::mt19937 &gen) {
  int id = generate_id();
  std::uniform_int_distribution<int> priority(1, 5);
  std::uniform_int_distribution<int> hour(0, 23);
  std::uniform_int_distribution<int> minute(0, 59);
  std::uniform_real_distribution<double> lat(b.lat_west, b.lat_east);
  std::uniform_real_distribution<double> lon(b.lon_north, b.lon_south);

  Call c;
  c.id = id;

  c.hour = hour(gen);
  c.minute = minute(gen);

  int p = priority(gen);
  std::string d = "";
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
      c.priority = CallPriority::Unknown;
  }

  Location l = {
    lat(gen),
    lon(gen)
  };

  c.location = l;

  return c;
}
