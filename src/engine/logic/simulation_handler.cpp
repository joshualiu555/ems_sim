#include <unordered_map>

#include "simulation_handler.hpp"
#include "simulation.hpp"

SimulationHandler::SimulationHandler(
  Postgres &db
) :
  db(db)
{}

int SimulationHandler::add_simulation() {
  int id = db.create_simulation();
  simulations[id] = std::make_unique<Simulation>(id, db);
  simulations[id] -> init(30, 30, 5, 5);
  return id;
}

void SimulationHandler::remove_simulation(int id) {
    simulations.erase(id);
}

Simulation* SimulationHandler::get_simulation(int id) {
    auto it = simulations.find(id);
    if (it == simulations.end()) {
        return nullptr;
    }
    return it -> second.get();
}
