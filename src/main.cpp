#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_map>

#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/call.hpp"

#include "logic/dispatch.hpp"
#include "logic/simulation.hpp"
#include "util/generate.hpp"
#include "io/output.hpp"
#include "io/logs.hpp"

#include "config/config.hpp"
#include "db/postgres.hpp"

int main() {
  std::cout << "EMS Dispatch System Simulator" << std::endl;

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
  std::unordered_map<int, Call> calls;
  for (int i = 0; i < 20; i++) {
    Call c = generate_call(b, gen);
    calls[c.id] = c;
  }

  Postgres db(get_connection_url());

  db.run_migrations("db/migrations");

  db.execute("TRUNCATE TABLE events, dispatches, calls, ambulances, hospitals RESTART IDENTITY CASCADE;");

  for (const auto& [id, a] : ambulances) {
    db.insert_ambulance(a);
  }
  for (const auto& [id, h] : hospitals) {
    db.insert_hospital(h);
  }
  for (const auto& [id, c] : calls) {
    db.insert_call(c);
  }

  Simulation simulation(calls, ambulances, hospitals, db);
  simulation.run();
}
