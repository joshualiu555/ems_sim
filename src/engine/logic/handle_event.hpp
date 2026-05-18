#pragma once

#include <vector> 
#include <unordered_map>

#include "simulation.hpp"

#include "models/call.hpp"
#include "models/event.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"

#include "db/postgres.hpp"

std::vector<Event> handle_call_received(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  const std::unordered_map<int, Hospital> &hospitals,
  Map &map,
  Postgres &db
);
std::vector<Event> handle_ambulance_arrive_at_scene(
  const Event &e, 
  const std::unordered_map<int, Call> &calls,
  std::unordered_map<int, Ambulance> &ambulances, 
  Postgres &db
);
std::vector<Event> handle_transport_start(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  const std::unordered_map<int, Hospital> &hospitals,
  Map &map,
  Postgres &db
);
std::vector<Event> handle_ambulance_arrive_at_hospital(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  Map &map,
  Postgres &db
);
std::vector<Event> handle_ambulance_back_at_station(
  const Event &e, 
  std::unordered_map<int, Ambulance> &ambulances,
  Postgres &db
);
std::vector<Event> handle_patient_discharged(
  const Event &e, 
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
);
std::vector<Event> handle_ambulance_move(
  const Event &e, 
  std::unordered_map<int, Call> &calls,
  std::unordered_map<int, Ambulance> &ambulances,
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
);
