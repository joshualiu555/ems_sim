#pragma once

#include <vector>
#include <optional>
#include <unordered_map>

#include "map.hpp"

#include "models/ambulance.hpp"
#include "models/dispatch.hpp"
#include "models/hospital.hpp"
#include "models/call.hpp"

std::optional<Dispatch> create_dispatch(
  const Call &c, 
  const std::unordered_map<int, Ambulance> &all_ambulances, 
  const std::unordered_map<int, Hospital> &all_hospitals,
  Map &map
);
std::vector<Ambulance> find_available_ambulances(
  const Call &c, 
  const std::unordered_map<int, Ambulance> &all_ambulances
);
std::vector<Hospital> find_available_hospitals(
  const std::unordered_map<int, Hospital> &all_hospitals
);
Ambulance find_closest_ambulance(
  const Call &c, 
  const std::vector<Ambulance> &available_ambulances,
  Map &map
);
Hospital find_closest_hospital(
  const Call &c, 
  const std::vector<Hospital> &available_hospitals,
  Map &map
);
