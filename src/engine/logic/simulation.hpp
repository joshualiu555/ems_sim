#pragma once

#include <queue>
#include <unordered_map>
#include <optional>
#include <random>

#include "models/call.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/event.hpp"

#include "map.hpp"

#include "db/postgres.hpp"

class Simulation {
  public:
    int current_time = 0;

    Simulation(
      int id,
      Postgres &db
    );

    void init(int width, int height, int num_ambulances, int num_hospitals);

    void add_call(Call &c);
    void run(int current_time);

    Cell get_random_road_cell();

  private:
    int id;

    std::unordered_map<int, Call> calls;
    std::unordered_map<int, Ambulance> ambulances;
    std::unordered_map<int, Hospital> hospitals;

    Postgres &db;


    std::priority_queue<Event> event_pq;
    std::priority_queue<Call, std::vector<Call>, std::greater<Call>> calls_pq;

    std::unique_ptr<Map> map;

    std::vector<Event> create_next_event(const Event &e);
};
 