#pragma once

#include <unordered_map>
#include <memory>

#include "simulation.hpp"
#include "db/postgres.hpp"

class SimulationHandler {
  public:
    SimulationHandler(Postgres &db);

    int add_simulation();
    void remove_simulation(int id);
    std::vector<int> get_all_simulations();
    Simulation *get_simulation(int id);

    void tick_all_simulations() {
    for (auto& [id, simulation_ptr] : simulations) {
        simulation_ptr -> run(simulation_ptr -> current_time);
        simulation_ptr -> current_time++;
    }
  }

  private:
    Postgres &db; 
    std::unordered_map<int, std::unique_ptr<Simulation>> simulations;
};