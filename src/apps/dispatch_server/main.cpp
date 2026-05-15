#include <iostream>
#include <unordered_map>
#include <chrono>

#include <asio.hpp>

#include "server.hpp"

#include "config/config.hpp"
#include "db/postgres.hpp"

#include "models/ambulance.hpp"
#include "models/hospital.hpp"

#include <util/generate.hpp>

int main() {
  Bounds b = {-30, 30, -30, 30};
  std::random_device rd;
  std::mt19937 gen(rd());

  std::unordered_map<int, Ambulance> ambulances;
  for (int i = 0; i < 5; i++) {
    Ambulance a = generate_ambulance(b, gen);
    ambulances[a.id] = a;
  }
  std::unordered_map<int, Hospital> hospitals;
  for (int i = 0; i < 5; i++) {
    Hospital h = generate_hospital(b, gen);
    hospitals[h.id] = h;
  }

  Postgres db(get_connection_url());
  db.run_migrations(MIGRATION_PATH);
  db.execute("TRUNCATE TABLE events, dispatches, calls, ambulances, hospitals RESTART IDENTITY CASCADE;");

  for (const auto& [id, a] : ambulances) {
    db.insert_ambulance(a);
  }
  for (const auto& [id, h] : hospitals) {
    db.insert_hospital(h);
  }

  Simulation simulation(ambulances, hospitals, db);

  asio::io_context io_context;
  Server server(io_context, 8080, ambulances, hospitals, db, simulation);    

  asio::steady_timer sim_timer(io_context, asio::chrono::seconds(0));
  std::function<void(const asio::error_code&)> tick_simulation;
  tick_simulation = [&](const asio::error_code& ec) {
    if (!ec) {
      simulation.run(simulation.current_time);

      simulation.current_time++;
      
      sim_timer.expires_at(sim_timer.expiry() + asio::chrono::seconds(1));
      sim_timer.async_wait(tick_simulation);
    }
  };
  sim_timer.async_wait(tick_simulation);

  io_context.run(); 

  return 0;
}
