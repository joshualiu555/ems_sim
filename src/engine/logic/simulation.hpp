#pragma once

#include <queue>
#include <unordered_map>
#include <optional>

#include "models/call.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/event.hpp"

#include "db/postgres.hpp"

class Simulation {
  public:
    int current_time = 0;

    Simulation(
      std::unordered_map<int, Ambulance> &ambulances, 
      std::unordered_map<int, Hospital> &hospitals,
      Postgres &db
    );

    void add_call(Call &c);
    void run(int current_time);

  private:
    std::unordered_map<int, Call> calls;
    std::unordered_map<int, Ambulance> &ambulances;
    std::unordered_map<int, Hospital> &hospitals;

    Postgres &db;

    std::priority_queue<Event> pq;

    std::optional<Event> create_next_event(Event &e);
};
 