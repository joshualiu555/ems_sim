#pragma once

#include <optional>
#include <unordered_map>

#include "models/call.hpp"
#include "models/event.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"

#include "db/postgres.hpp"

std::optional<Event> handle_call_received(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  const std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
);
std::optional<Event> handle_ambulance_arrive_at_scene(
  const Event &e, 
  const std::unordered_map<int, Call> &calls
);
std::optional<Event> handle_transport_start(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  const std::unordered_map<int, Hospital> &hospitals
);
std::optional<Event> handle_ambulance_arrive_at_hospital(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  const std::unordered_map<int, Ambulance> &ambulances, 
  const std::unordered_map<int, Hospital> &hospitals
);
std::optional<Event> handle_ambulance_back_at_station(
  const Event &e, 
  std::unordered_map<int, Ambulance> &ambulances
);
