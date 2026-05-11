#pragma once

#include <optional>
#include <unordered_map>

#include "models/call.hpp"
#include "models/event.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"

#include "db/postgres.hpp"

std::optional<Event> handle_call_received(
  Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
);
std::optional<Event> handle_ambulance_arrive_at_scene(
  Event &e, 
  std::unordered_map<int, Call> &calls
);
std::optional<Event> handle_transport_start(
  Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Hospital> &hospitals
);
std::optional<Event> handle_ambulance_arrive_at_hospital(
  Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals
);
std::optional<Event> handle_ambulance_back_at_station(
  Event &e, 
  std::unordered_map<int, Ambulance> &ambulances
);
