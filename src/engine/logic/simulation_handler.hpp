#pragma once

#include <unordered_map>
#include <memory>

#include "simulation.hpp"
#include "db/postgres.hpp"

class SimulationHandler {
  public:
    SimulationHandler(Postgres &db);

    void pause();
    void resume();
    void set_speed(double multiplier);
    bool is_paused();
    int get_tick_interval_ms();      

    int add_simulation();
    int add_custom_simulation(const std::string& grid);
    void remove_simulation(int id);
    std::vector<int> get_all_simulations();
    Simulation *get_simulation(int id);

    int save_simulation(int sim_id);
    int restore_simulation(int saved_id);
    std::vector<int> get_saved_simulations();

    void tick_all_simulations();

  private:
    Postgres &db; 
    std::unordered_map<int, std::unique_ptr<Simulation>> simulations;

    bool paused = false;
    int tick_interval_ms = 1000;
};