#include <vector>
#include <optional>
#include <unordered_map>

#include "models/ambulance.hpp"
#include "models/dispatch.hpp"
#include "models/hospital.hpp"
#include "models/call.hpp"

#include "dispatch.hpp"
#include "utility.hpp"

std::optional<Dispatch> create_dispatch(Call &c, std::unordered_map<int, Ambulance> &all_ambulances, std::unordered_map<int, Hospital> &all_hospitals) {
    auto available_ambulances = find_available_ambulances(c, all_ambulances);
    if (available_ambulances.empty()) {
        return std::nullopt;
    }

    Ambulance best_ambulance = find_closest_ambulance(c, available_ambulances);

    auto available_hospitals = find_available_hospitals(all_hospitals);
    if (available_hospitals.empty()) {
        return std::nullopt;
    }

    Hospital best_hospital = find_closest_hospital(c, available_hospitals);

    return Dispatch{
        c.id,
        best_ambulance.id,
        best_hospital.id
    };
}

std::vector<Ambulance> find_available_ambulances(Call &c, std::unordered_map<int, Ambulance> &all_ambulances) {
  std::vector<Ambulance> available_ambulances; 
  for (const auto& [id, a] : all_ambulances) {
      if (a.ambulance_status == AmbulanceStatus::Available) {
        if (a.ambulance_type == AmbulanceType::ALS ||
          (a.ambulance_type == AmbulanceType::BLS &&
          (c.priority == CallPriority::Alpha || c.priority == CallPriority::Bravo))) {

          available_ambulances.push_back(a);
        }
      }
    }
  return available_ambulances;
}

std::vector<Hospital> find_available_hospitals(std::unordered_map<int, Hospital> &all_hospitals) {
  std::vector<Hospital> available_hospitals; 
  for (const auto& [id, h] : all_hospitals) {
    if (h.num_patients < h.capacity) {
      available_hospitals.push_back(h);
    }
  }
  return available_hospitals;
}

Ambulance find_closest_ambulance(Call &c, std::vector<Ambulance> &available_ambulances) {
  Ambulance closest_ambulance;
  int min_distance = INT_MAX;
  for (Ambulance a : available_ambulances) {
    int d = find_distance(a.location, c.location);
    if (d < min_distance) {
      closest_ambulance = a;
      min_distance = d;
    }
  }
  return closest_ambulance;
}

Hospital find_closest_hospital(Call &c, std::vector<Hospital> &available_hospitals) {
  Hospital closest_hospital;
  int min_distance = INT_MAX;
  for (Hospital h : available_hospitals) {
    int d = find_distance(h.location, c.location);
    if (d < min_distance) {
      closest_hospital = h;
      min_distance = d;
    }
  }
  return closest_hospital;
}
