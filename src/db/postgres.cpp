#include <optional>
#include <fstream>
#include <filesystem>

#include <pqxx/pqxx>

#include "postgres.hpp"

#include "../util/convert.hpp" 

Postgres::Postgres(const std::string& conn_str) : conn(conn_str) {}

void Postgres::execute(const std::string &sql) {
  pqxx::work txn(conn);
  txn.exec(sql);
  txn.commit();
}

pqxx::result Postgres::query(const std::string &sql) {
  pqxx::work txn(conn);
  auto result = txn.exec(sql);
  txn.commit();
  return result;
}

bool Postgres::check_if_migration_exists(const std::string &file) {
  pqxx::read_transaction txn(conn);
    
  pqxx::result exists = txn.exec(
    "SELECT 1 FROM schema_migrations WHERE version = $1",
    pqxx::params(file)
  );

  return !exists.empty();
}

void Postgres::run_migrations(const std::string &dir) {
  execute(
    "CREATE TABLE IF NOT EXISTS schema_migrations (version TEXT PRIMARY KEY);"
  );

  std::vector<std::filesystem::path> migration_files;
  for (const auto &file : std::filesystem::directory_iterator(dir)) {
    migration_files.push_back(file.path());
  }
  std::sort(migration_files.begin(), migration_files.end());

  for (const auto &file : migration_files) {
    std::string filename = file.filename().string();

    if (!check_if_migration_exists(filename)) {
      std::ifstream ifs(file);
      std::string sql((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));

      // not using defined functions because these two queries must be one transaction
      pqxx::work txn(conn);
      txn.exec(sql);
      txn.exec(
          "INSERT INTO schema_migrations (version) VALUES ($1);",
          pqxx::params(filename)
      );
      txn.commit();
    }
  }
}

void Postgres::insert_hospital(const Hospital &h) {
  execute_params(
    "INSERT INTO hospitals (id, num_patients, capacity, lat, lon) VALUES ($1, $2, $3, $4, $5)",
    h.id,
    h.num_patients,
    h.capacity,
    h.location.lat,
    h.location.lon
  );
}

void Postgres::insert_ambulance(const Ambulance &a) {
  execute_params(
    "INSERT INTO ambulances (id, status, type, lat, lon) VALUES ($1, $2, $3, $4, $5)",
    a.id,
    to_string(a.ambulance_status),
    to_string(a.ambulance_type),
    a.location.lat,
    a.location.lon
  );
}

void Postgres::insert_call(const Call &c) {
  execute_params(
    "INSERT INTO calls (id, call_hour, call_minute, priority, description, lat, lon) VALUES ($1, $2, $3, $4, $5, $6, $7)",
    c.id,
    c.time.hour,
    c.time.minute,
    to_string(c.priority),
    c.description,
    c.location.lat,
    c.location.lon
  );
}

void Postgres::insert_dispatch(const Dispatch &d) {
  execute_params(
    "INSERT INTO dispatches (call_id, ambulance_id, hospital_id) VALUES ($1, $2, $3)",
    d.call_id,
    d.ambulance_id,
    d.hospital_id
  );
}

void Postgres::insert_event(const Event &e) {
  std::optional<int> c_id = e.call_id == -1 ? std::nullopt : std::optional<int>(e.call_id);
  std::optional<int> a_id = e.ambulance_id == -1 ? std::nullopt : std::optional<int>(e.ambulance_id);
  std::optional<int> h_id = e.hospital_id == -1 ? std::nullopt : std::optional<int>(e.hospital_id);

  execute_params(
    "INSERT INTO events (event_hour, event_minute, event_type, call_id, ambulance_id, hospital_id) VALUES ($1, $2, $3, $4, $5, $6)",
    e.time.hour,
    e.time.minute,
    to_string(e.event_type),
    c_id,
    a_id,
    h_id
  );
}
