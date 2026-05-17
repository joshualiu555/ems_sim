#pragma once

#include <queue>
#include <unordered_map>
#include <optional>
#include <random>

#include "models/call.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/event.hpp"
#include "models/map.hpp"

#include "db/postgres.hpp"

class Map {
  public:
    Map(
      int width,
      int height
    );

    void init_map();
    Cell place_medical_cells(CellType ct, std::mt19937 &gen);
    // connect medical points
    void create_main_roads();
    // one-off roads
    void create_side_roads(double branch_chance, int min_length, int max_length, std::mt19937 &gen);
    void print_map();

    std::string serialize();
    void deserialize(const std::string &layout);

  private:
    int width;
    int height;
    std::vector<std::vector<Cell>> map;
    std::vector<Cell> medical_cells;

    // draw L-shaped road
    void draw_road(Cell a, Cell b);
};

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

  private:
    int id;

    std::unordered_map<int, Call> calls;
    std::unordered_map<int, Ambulance> ambulances;
    std::unordered_map<int, Hospital> hospitals;

    Postgres &db;

    std::priority_queue<Event> pq;

    std::unique_ptr<Map> map;

    std::vector<Event> create_next_event(const Event &e);
};
 