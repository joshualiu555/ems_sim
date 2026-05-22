#include <iostream>
#include <asio.hpp>

#include "server.hpp"
#include "db/config.hpp"
#include "db/postgres.hpp"
#include "logic/simulation_handler.hpp"

int main() {
  Postgres db(get_connection_url());
  db.run_migrations(MIGRATION_PATH);

  db.execute("TRUNCATE TABLE simulations, events, dispatches, calls, ambulances, hospitals RESTART IDENTITY CASCADE;");

  SimulationHandler simulation_handler(db);

  asio::io_context io_context;
  Server server(io_context, 8080, simulation_handler);    

  asio::steady_timer simulation_timer(io_context, asio::chrono::seconds(0));
  std::function<void(const asio::error_code&)> tick_simulations;
  
  tick_simulations = [&](const asio::error_code& ec) {
    if (!ec) {
      if (!simulation_handler.is_paused()) {
        simulation_handler.tick_all_simulations();
      }

      simulation_timer.expires_at(
        simulation_timer.expiry() +
        asio::chrono::milliseconds(simulation_handler.get_tick_interval_ms())
      );
      simulation_timer.async_wait(tick_simulations);
    }
  };

  simulation_timer.async_wait(tick_simulations);

  io_context.run(); 

  return 0;
}
