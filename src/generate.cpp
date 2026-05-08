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
#include "utility.hpp"

int ambulance_id = 0;
int hospital_id = 0;
int call_id = 0;

Time current_time = {0, 0};

int generate_id(std::string s) {
  if (s == "ambulance") {
    return ambulance_id++;
  } else if (s == "hospital") {
    return hospital_id++;
  } else {
    return call_id++;
  }
}

Ambulance generate_ambulance(Bounds &b, std::mt19937 &gen) {
  int id = generate_id("ambulance");
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

Hospital generate_hospital(Bounds &b, std::mt19937 &gen) {
  int id = generate_id("hospital");
  std::uniform_real_distribution<double> lat(b.lat_west, b.lat_east);
  std::uniform_real_distribution<double> lon(b.lon_north, b.lon_south);
  std::uniform_int_distribution<int> capacity(1, 5);

  int c = capacity(gen);
  Location l = {
    lat(gen),
    lon(gen)
  };

  Hospital h = {
    id,
    0,
    c,
    l
  };

  return h;
}

Call generate_call(Bounds &b, std::mt19937 &gen) {
  int id = generate_id("call");
  std::uniform_int_distribution<int> priority(1, 5);
  std::uniform_int_distribution<int> time_gap(1, 10);
  std::uniform_real_distribution<double> lat(b.lat_west, b.lat_east);
  std::uniform_real_distribution<double> lon(b.lon_north, b.lon_south);

  Call c;
  c.id = id;

  current_time = find_next_time(current_time, time_gap(gen));
  c.time = current_time;

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
  }

  Location l = {
    lat(gen),
    lon(gen)
  };

  c.location = l;

  return c;
}
