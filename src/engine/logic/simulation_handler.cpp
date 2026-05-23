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

int SimulationHandler::add_custom_simulation(const std::string& grid) {
  int id = db.create_simulation();
  simulations[id] = std::make_unique<Simulation>(id, db);
  simulations[id] -> init_custom(grid);
  return id;
}

void SimulationHandler::remove_simulation(int id) {
  simulations.erase(id);
  db.delete_simulation(id);
}

std::vector<int> SimulationHandler::get_all_simulations() {
  return db.get_all_simulations();
}

Simulation* SimulationHandler::get_simulation(int id) {
  auto it = simulations.find(id);
  if (it == simulations.end()) {
    return nullptr;
  }
  return it -> second.get();
}

void SimulationHandler::pause() { 
  paused = true; 
}
void SimulationHandler::resume() { 
  paused = false; 
}
bool SimulationHandler::is_paused() { 
  return paused; 
}
int SimulationHandler::get_tick_interval_ms() { 
  return tick_interval_ms; 
}

void SimulationHandler::set_speed(double multiplier) {
  tick_interval_ms = static_cast<int>(1000.0 / multiplier);
}

int SimulationHandler::save_simulation(int sim_id) {
  return db.save_simulation(sim_id);
}

int SimulationHandler::restore_simulation(int saved_id) {
  int id = db.create_simulation();
  simulations[id] = std::make_unique<Simulation>(id, db);
  simulations[id] -> init_from_save(saved_id);
  return id;
}

std::vector<int> SimulationHandler::get_saved_simulations() {
  return db.get_saved_simulations();
}

void SimulationHandler::tick_all_simulations() {
for (auto& [id, simulation_ptr] : simulations) {
  simulation_ptr -> run(simulation_ptr -> current_time);
  simulation_ptr -> current_time++;
}
}