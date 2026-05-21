#pragma once

#include <vector> 
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include "simulation.hpp"
#include "models/call.hpp"
#include "models/event.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "db/postgres.hpp"

std::vector<Event> process_calls(
  int simulation_id,
  int current_time,
  std::unordered_map<int, Call> &calls,
  std::unordered_map<int, Ambulance> &ambulances,
  std::unordered_map<int, Hospital> &hospitals,
  std::priority_queue<Call, std::vector<Call>, std::greater<Call>> &pending_calls,
  Map &map,
  Postgres &db
);

std::vector<Event> handle_call_received(
  const Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  std::priority_queue<Call, std::vector<Call>, std::greater<Call>> &pending_calls,
  Map &map,
  Postgres &db
);

std::vector<Event> handle_ambulance_arrive_at_scene(
  const Event &e, 
  std::unordered_map<int, Call> &calls,
  std::unordered_set<int> &pending_call_ids,
  std::unordered_map<int, Ambulance> &ambulances, 
  Postgres &db
);

std::vector<Event> handle_transport_start(
  const Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  const std::unordered_map<int, Hospital> &hospitals,
  Map &map,
  Postgres &db
);

std::vector<Event> handle_ambulance_arrive_at_hospital(
  const Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  std::priority_queue<Call, std::vector<Call>, std::greater<Call>> &calls_pq,
  Map &map,
  Postgres &db
);

std::vector<Event> handle_ambulance_back_at_station();

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

std::vector<Event> handle_call_expired(
  const Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  std::priority_queue<Call, std::vector<Call>, std::greater<Call>> &pending_calls,
  std::unordered_set<int> &pending_call_ids,
  std::unordered_set<int> &expired_call_ids,
  Map &map,
  Postgres &db
);
