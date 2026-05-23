#pragma once

#include <queue>
#include <unordered_map>
#include <optional>
#include <random>
#include <vector>
#include <unordered_set>

#include <nlohmann/json.hpp>

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
    void init_custom(const std::string& grid);
    void init_from_save(int saved_id);

    void add_call(Call &c);
    void run(int current_time);

    Cell get_random_road_cell();

    std::vector<Cell> get_current_map();
    std::unordered_map<int, Ambulance> get_ambulances();
    std::vector<Cell> get_static_map();

    nlohmann::json get_analytics(int simulation_id);

  private:
    int id;

    std::unordered_map<int, Call> calls;
    std::unordered_set<int> pending_call_ids;
    std::unordered_set<int> expired_call_ids;
    std::unordered_set<int> patient_discharged_ids;
    std::unordered_map<int, Ambulance> ambulances;
    std::unordered_map<int, Hospital> hospitals;

    Postgres &db;

    std::priority_queue<Event> event_pq;
    std::priority_queue<Call, std::vector<Call>, std::greater<Call>> calls_pq;

    std::unique_ptr<Map> map;

    std::vector<Event> create_next_event(const Event &e);
};
 