#pragma once

#include <vector>
#include <optional>
#include <unordered_map>

#include "models.hpp"

std::optional<Dispatch> create_dispatch(Call &c, std::unordered_map<int, Ambulance> &all_ambulances, std::unordered_map<int, Hospital> &all_hospitals);
std::vector<Ambulance> find_available_ambulances(Call &c, std::unordered_map<int, Ambulance> &all_ambulances);
Ambulance find_closest_ambulance(Call &c, std::vector<Ambulance> &available_ambulances);
std::vector<Hospital> find_available_hospitals(std::unordered_map<int, Hospital> &all_hospitals);
Hospital find_closest_hospital(Call &c, std::vector<Hospital> &available_hospitals);