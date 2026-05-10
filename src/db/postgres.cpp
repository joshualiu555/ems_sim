#include <optional>

#include <pqxx/pqxx>

#include "postgres.hpp"

#include "../util/convert.hpp" 

Postgres::Postgres(const std::string& conn_str) : conn(conn_str) {}

void Postgres::execute(const std::string& sql) {
  pqxx::work txn(conn);
  txn.exec(sql);
  txn.commit();
}

pqxx::result Postgres::query(const std::string& sql) {
  pqxx::work txn(conn);
  auto result = txn.exec(sql);
  txn.commit();
  return result;
}

void Postgres::insert_hospital(const Hospital &h) {
  pqxx::work txn(conn);
  txn.exec(
    "INSERT INTO hospitals (id, num_patients, capacity, lat, lon) VALUES ($1, $2, $3, $4, $5)",
    pqxx::params{
      h.id,
      h.num_patients,
      h.capacity,
      h.location.lat,
      h.location.lon
    }
  );
  txn.commit();
}

void Postgres::insert_ambulance(const Ambulance &a) {
  pqxx::work txn(conn);
  txn.exec(
    "INSERT INTO ambulances (id, status, type, lat, lon) VALUES ($1, $2, $3, $4, $5)",
    pqxx::params{
      a.id,
      to_string(a.ambulance_status),
      to_string(a.ambulance_type),
      a.location.lat,
      a.location.lon
    }
  );
  txn.commit();
}

void Postgres::insert_call(const Call &c) {
  pqxx::work txn(conn);
  txn.exec(
    "INSERT INTO calls (id, call_hour, call_minute, priority, description, lat, lon) VALUES ($1, $2, $3, $4, $5, $6, $7)",
    pqxx::params{
      c.id,
      c.time.hour,
      c.time.minute,
      to_string(c.priority),
      c.description,
      c.location.lat,
      c.location.lon
    }
  );
  txn.commit();
}

void Postgres::insert_dispatch(const Dispatch &d) {
  pqxx::work txn(conn);
  txn.exec(
    "INSERT INTO dispatches (call_id, ambulance_id, hospital_id) VALUES ($1, $2, $3)",
    pqxx::params{
      d.call_id,
      d.ambulance_id,
      d.hospital_id
    }
  );
  txn.commit();
}

void Postgres::insert_event(const Event &e) {
  pqxx::work txn(conn);

  std::optional<int> c_id = e.call_id == -1 ? std::nullopt : std::optional<int>(e.call_id);
  std::optional<int> a_id = e.ambulance_id == -1 ? std::nullopt : std::optional<int>(e.ambulance_id);
  std::optional<int> h_id = e.hospital_id == -1 ? std::nullopt : std::optional<int>(e.hospital_id);

  txn.exec(
    "INSERT INTO events (event_hour, event_minute, event_type, call_id, ambulance_id, hospital_id) VALUES ($1, $2, $3, $4, $5, $6)",
    pqxx::params{
      e.time.hour,
      e.time.minute,
      to_string(e.event_type),
      c_id,
      a_id,
      h_id
    }
  );

  txn.commit();
}
