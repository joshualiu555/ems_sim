#include <iostream>
#include <unordered_map>

#include <asio.hpp>

#include "server.hpp"

#include "config/config.hpp"
#include "db/postgres.hpp"

#include "models/ambulance.hpp"
#include "models/hospital.hpp"

#include <util/generate.hpp>

int main() {
  Bounds b = {-100, 100, -100, 100};
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

  asio::io_context io_context;
  Server server(io_context, 8080, ambulances, hospitals, db);    
  io_context.run(); 

  return 0;
}
