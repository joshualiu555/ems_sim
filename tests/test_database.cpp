#include <memory>

#include <gtest/gtest.h>
#include <pqxx/pqxx> 

#include "db/postgres.hpp"
#include "db/config.hpp"

#include "models/call.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/event.hpp"

class DatabaseTest:public::testing::Test {
  protected:
    std::unique_ptr<Postgres> db;
    int simulation_id = 1;

    void SetUp() override {
      db = std::make_unique<Postgres>(get_connection_url());
      db -> run_migrations(MIGRATION_PATH);
      db -> execute("TRUNCATE simulations, calls, ambulances, hospitals, dispatches, events RESTART IDENTITY CASCADE;");
      db -> execute("INSERT INTO simulations (id) VALUES (1);");
    }
};

TEST_F(DatabaseTest, Insert) {
  Call c = {1, simulation_id, 0, CallPriority::Alpha, "", {0, 0}};
  Ambulance a = {1, simulation_id, AmbulanceStatus::Available, AmbulanceType::ALS, {0, 0}, {0, 0}};
  Hospital h = {1, simulation_id, 0, 10, {0, 0}};

  EXPECT_NO_THROW({
    db -> insert_call(c);
    db -> insert_ambulance(a);
    db -> insert_hospital(h);
  });
}

TEST_F(DatabaseTest, Update) {
  Ambulance a = {1, simulation_id, AmbulanceStatus::Available, AmbulanceType::ALS, {0, 0}, {0, 0}};
  Hospital h = {1, simulation_id, 0, 10, {0, 0}};
  db -> insert_ambulance(a);
  db->insert_hospital(h);

  EXPECT_NO_THROW({
    db -> update_ambulance_status("Transporting", 1);
    db -> update_ambulance_location(0, 0, 1);
    db -> update_hospital(1, 1);
  });
}

TEST_F(DatabaseTest, ForeignKeyViolation) {
  Event e = {simulation_id, 0, EventType::CallReceived, 999, -1, -1};

  EXPECT_THROW({
    db -> insert_event(e);
  }, pqxx::foreign_key_violation);
}

TEST_F(DatabaseTest, UniqueViolation) {
  EXPECT_THROW({
    db -> execute("INSERT INTO schema_migrations (version) VALUES ('001_init.sql');");
    db -> execute("INSERT INTO schema_migrations (version) VALUES ('001_init.sql');"); 
  }, pqxx::unique_violation);
}
