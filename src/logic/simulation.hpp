#pragma once

#include <queue>
#include <unordered_map>
#include <optional>

#include "../models/call.hpp"
#include "../models/ambulance.hpp"
#include "../models/hospital.hpp"
#include "../models/event.hpp"

class Simulation {
public:
  Simulation(std::unordered_map<int, Call> &calls, std::unordered_map<int, Ambulance> &ambulances, std::unordered_map<int, Hospital> &hospitals);

  void run();

private:
  std::unordered_map<int, Call> &calls;
  std::unordered_map<int, Ambulance> &ambulances;
  std::unordered_map<int, Hospital> &hospitals;

  std::priority_queue<Event> pq;

  void init();

  std::optional<Event> create_next_event(Event &e);
};
