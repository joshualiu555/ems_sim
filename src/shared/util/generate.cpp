#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <string>

#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/call.hpp"


#include "generate.hpp"

Ambulance generate_ambulance(const Bounds &b, std::mt19937 &gen, int simulation_id) {
  std::uniform_int_distribution<int> ambulance_type(1, 3);
  std::uniform_int_distribution<int> x(b.x_min, b.x_max);
  std::uniform_int_distribution<int> y(b.y_min, b.y_max);

  AmbulanceStatus as = AmbulanceStatus::Available;

  int ambulance_type_range = ambulance_type(gen);
  AmbulanceType at = (ambulance_type_range == 1) ? AmbulanceType::BLS : AmbulanceType::ALS;

  Ambulance a = {
    0, // dummy id that will later be replaced by the database, the source of truth
    simulation_id,
    as,
    at,
    x(gen),
    y(gen),
    x(gen),
    y(gen)
  };

  return a;
}

Hospital generate_hospital(const Bounds &b, std::mt19937 &gen, int simulation_id) {
  std::uniform_int_distribution<int> x(b.x_min, b.x_max);
  std::uniform_int_distribution<int> y(b.y_min, b.y_max);
  std::uniform_int_distribution<int> capacity(1, 5);

  int c = capacity(gen);

  Hospital h = {
    0,
    simulation_id,
    0,
    c,
    x(gen),
    y(gen)
  };

  return h;
}

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
