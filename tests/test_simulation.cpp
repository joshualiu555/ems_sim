#include <unordered_map>
#include <memory>

#include <gtest/gtest.h>

#include "logic/simulation.hpp"
#include "logic/handle_event.hpp"

#include "models/call.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"

#include "config/config.hpp"
#include "db/postgres.hpp"

class HandleEventTest:public::testing::Test {
protected:
  std::unique_ptr<Postgres> db;
  std::unordered_map<int, Call> calls;
  std::unordered_map<int, Ambulance> ambulances;
  std::unordered_map<int, Hospital> hospitals;

  void SetUp() override {
    calls = {{1, {1, 0, CallPriority::Alpha, "", {0, 0}}}};
    ambulances = {{1, {1, AmbulanceStatus::Available, AmbulanceType::BLS, {0, 0}}}};
    hospitals = {{1, {1, 0, 10, {0, 0}}}};

    db = std::make_unique<Postgres>(get_connection_url());
    db -> run_migrations(MIGRATION_PATH);
    db -> execute("TRUNCATE calls, ambulances, hospitals, dispatches, events RESTART IDENTITY CASCADE;");
  }

  void init_db(AmbulanceStatus ambulance_status) {
    db -> execute("INSERT INTO calls (id, call_time, priority, description, lat, lon) VALUES (1, 0, 'Alpha', 'Test Call', 0.0, 0.0);");
    db -> execute("INSERT INTO hospitals (id, capacity, lat, lon) VALUES (1, 10, 0.0, 0.0);");
    
    std::string status = (ambulance_status == AmbulanceStatus::Available) ? "Available" : "Transporting";
    db -> execute_params(
      "INSERT INTO ambulances (id, status, type, lat, lon) VALUES ($1, $2, $3, $4, $5);",
      1,
      status,
      "BLS",
      0.0,
      0.0
    );
  }
};

TEST_F(HandleEventTest, CompleteCall) {
  init_db(AmbulanceStatus::Available);

  Simulation simulation(ambulances, hospitals, *db);
  simulation.run(0);

  EXPECT_EQ(ambulances[1].ambulance_status, AmbulanceStatus::Available);
}

TEST_F(HandleEventTest, CallReceivedToAmbulanceArriveAtScene) {
  init_db(AmbulanceStatus::Available);

  Event e = {0, EventType::CallReceived, 1, -1, -1};
  std::optional<Event> next_event = handle_call_received(e, calls, ambulances, hospitals, *db);

  ASSERT_TRUE(next_event);
  EXPECT_EQ(next_event -> event_type, EventType::AmbulanceArriveAtScene);
  EXPECT_EQ(next_event -> ambulance_id, 1);
  EXPECT_EQ(next_event -> hospital_id, 1);
  EXPECT_EQ(ambulances[next_event -> ambulance_id].ambulance_status, AmbulanceStatus::Transporting);
}

TEST_F(HandleEventTest, CallReceivedFailure) {
  ambulances[1].ambulance_status = AmbulanceStatus::Transporting;
  init_db(AmbulanceStatus::Transporting);

  Event e = {0, EventType::CallReceived, 1, -1, -1};
  std::optional<Event> next_event = handle_call_received(e, calls, ambulances, hospitals, *db);

  EXPECT_FALSE(next_event);
}

TEST_F(HandleEventTest, AmbulanceArriveAtSceneToTransportStart) {
  Event e = {0, EventType::AmbulanceArriveAtScene, 1, 1, 1};
  std::optional<Event> next_event = handle_ambulance_arrive_at_scene(e, calls);

  ASSERT_TRUE(next_event);
  EXPECT_EQ(next_event -> event_type, EventType::TransportStart);
  EXPECT_EQ(next_event -> time, 5);
}

TEST_F(HandleEventTest, TransportStartToAmbulanceArriveAtHospital) {
  Event e = {0, EventType::TransportStart, 1, 1, 1};
  std::optional<Event> next = handle_transport_start(e, calls, hospitals);

  ASSERT_TRUE(next);
  EXPECT_EQ(next -> event_type, EventType::AmbulanceArriveAtHospital);
}

TEST_F(HandleEventTest, AmbulanceArriveAtHospitalToAmulanceBackAtStation) {
  ambulances[1].ambulance_status = AmbulanceStatus::Transporting;
  Event e = {0, EventType::AmbulanceArriveAtHospital, 1, 1, 1};

  std::optional<Event> next = handle_ambulance_arrive_at_hospital(e, calls, ambulances, hospitals);

  ASSERT_TRUE(next);
  EXPECT_EQ(next -> event_type, EventType::AmbulanceBackAtStation);
}

TEST_F(HandleEventTest, BackAtStation) {
  ambulances[1].ambulance_status = AmbulanceStatus::Transporting;
  Event e = {0, EventType::AmbulanceBackAtStation, 1, 1, 1};
  
  std::optional<Event> next = handle_ambulance_back_at_station(e, ambulances);

  EXPECT_FALSE(next);
  EXPECT_EQ(ambulances[1].ambulance_status, AmbulanceStatus::Available);
}
