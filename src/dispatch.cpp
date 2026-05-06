#include <vector>
#include <optional>

#include "dispatch.hpp"
#include "models.hpp"
#include "distance.hpp"

std::optional<Dispatch> create_dispatch(Call c, std::vector<Ambulance> all_ambulances, std::vector<Hospital> all_hospitals) {
    std::vector<Ambulance> available_ambulances = find_available_ambulances(all_ambulances);
    if (available_ambulances.empty()) {
      return std::nullopt;
    }
    Ambulance best_ambulance = find_closest_ambulance(c, available_ambulances);

    std::vector<Hospital> available_hospitals = find_available_hospitals(all_hospitals);
    if (available_hospitals.empty()) {
      return std::nullopt;
    }
    Hospital best_hospital = find_closest_hospital(c, available_hospitals);

    Dispatch d = {
      c.id,
      best_ambulance.id,
      best_hospital.id
    };

    return d;
}

std::vector<Ambulance> find_available_ambulances(std::vector<Ambulance> all_ambulances) {
  std::vector<Ambulance> available_ambulances; 
  for (Ambulance a : all_ambulances) {
    if (a.ambulance_status == AmbulanceStatus::Available) {
      available_ambulances.push_back(a);
    }
  }
  return available_ambulances;
}

std::vector<Hospital> find_available_hospitals(std::vector<Hospital> all_hospitals) {
  std::vector<Hospital> available_hospitals; 
  for (Hospital h : all_hospitals) {
    if (h.num_patients < h.capacity) {
      available_hospitals.push_back(h);
    }
  }
  return available_hospitals;
}

Ambulance find_closest_ambulance(Call c, std::vector<Ambulance> available_ambulances) {
  Ambulance closest_ambulance;
  int min_distance = INT_MAX;
  for (Ambulance a : available_ambulances) {
    int d = distance(a.location, c.location);
    if (d < min_distance) {
      closest_ambulance = a;
      min_distance = d;
    }
  }
  return closest_ambulance;
}

Hospital find_closest_hospital(Call c, std::vector<Hospital> available_hospitals) {
  Hospital closest_hospital;
  int min_distance = INT_MAX;
  for (Hospital h : available_hospitals) {
    int d = distance(h.location, c.location);
    if (d < min_distance) {
      closest_hospital = h;
      min_distance = d;
    }
  }
  return closest_hospital;
}
