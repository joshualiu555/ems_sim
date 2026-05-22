#include <vector>
#include <optional>
#include <unordered_map>
#include <limits>
#include <climits>

#include "models/ambulance.hpp"
#include "models/dispatch.hpp"
#include "models/hospital.hpp"
#include "models/call.hpp"

#include "map.hpp"
#include "dispatch.hpp"

std::optional<Dispatch> create_dispatch(
  const Call &c, 
  const std::unordered_map<int, Ambulance> &all_ambulances, 
  const std::unordered_map<int, Hospital> &all_hospitals,
  Map &map
) {
    auto available_ambulances = find_available_ambulances(c, all_ambulances);
    if (available_ambulances.empty()) {
      return std::nullopt;
    }

    Ambulance best_ambulance = find_closest_ambulance(c, available_ambulances, map);

    auto available_hospitals = find_available_hospitals(all_hospitals);
    if (available_hospitals.empty()) {
      return std::nullopt;
    }

    Hospital best_hospital = find_closest_hospital(c, available_hospitals, map);

    return Dispatch{
      c.simulation_id,
      c.id,
      best_ambulance.id,
      best_hospital.id
    };
}

std::vector<Ambulance> find_available_ambulances(const Call &c, const std::unordered_map<int, Ambulance> &all_ambulances) {
  std::vector<Ambulance> available_ambulances; 
  for (const auto &[id, a] : all_ambulances) {
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

std::vector<Hospital> find_available_hospitals(const std::unordered_map<int, Hospital> &all_hospitals) {
  std::vector<Hospital> available_hospitals; 
  for (const auto &[id, h] : all_hospitals) {
    if (h.num_patients < h.capacity) {
      available_hospitals.push_back(h);
    }
  }
  return available_hospitals;
}

Ambulance find_closest_ambulance(const Call &c, const std::vector<Ambulance> &available_ambulances, Map &map) {
  Ambulance closest_ambulance = available_ambulances[0];
  int min_distance = INT_MAX;

  for (const Ambulance &a : available_ambulances) {
    Cell start = {a.current_x, a.current_y, CellType::Ambulance};
    Cell end = {c.x, c.y, CellType::Call};
    
    std::vector<Cell> path = map.find_path(start, end);
    if (!path.empty() || (a.current_x == c.x && a.current_y == c.y)) {
      int distance = path.size();
      if (distance < min_distance) {
        closest_ambulance = a;
        min_distance = distance;
      }
    }
  }

  return closest_ambulance;
}

Hospital find_closest_hospital(const Call &c, const std::vector<Hospital> &available_hospitals, Map &map) {
  Hospital closest_hospital = available_hospitals[0];
  int min_distance = INT_MAX;

  for (const Hospital &h : available_hospitals) {
    Cell start = {c.x, c.y, CellType::Call}; 
    Cell end = {h.x, h.y, CellType::Hospital};
    
    std::vector<Cell> path = map.find_path(start, end);
    if (!path.empty() || (c.x == h.x && c.y == h.y)) {
      int distance = path.size();
      if (distance < min_distance) {
        closest_hospital = h;
        min_distance = distance;
      }
    }
  }

  return closest_hospital;
}
