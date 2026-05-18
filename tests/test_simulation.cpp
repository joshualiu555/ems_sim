#include <unordered_map>
#include <memory>
#include <vector>
#include <optional>
#include <gtest/gtest.h>

#include "logic/simulation.hpp"
#include "logic/handle_event.hpp"

#include "models/call.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/map.hpp"

#include "db/config.hpp"
#include "db/postgres.hpp"

class HandleEventTest:public::testing::Test {
  protected:
    int simulation_id = 1;
    std::unique_ptr<Postgres> db;
    std::unordered_map<int, Call> calls;
    std::unordered_map<int, Ambulance> ambulances;
    std::unordered_map<int, Hospital> hospitals;
    std::unique_ptr<Map> map; 

    void SetUp() override {
      calls = {{1, {1, simulation_id, 0, CallPriority::Alpha, "", 0, 2}}};
      ambulances = {{1, {1, simulation_id, AmbulanceStatus::Available, AmbulanceType::BLS, 0, 0, 0, 0}}};
      hospitals = {{1, {1, simulation_id, 0, 10, 0, 4}}};

      map = std::make_unique<Map>(5, 5);
      map->deserialize("A###H....................");

      db = std::make_unique<Postgres>(get_connection_url());
      db -> run_migrations(MIGRATION_PATH);
      db -> execute("TRUNCATE simulations, calls, ambulances, hospitals, dispatches, events RESTART IDENTITY CASCADE;");
      db -> execute("INSERT INTO simulations (id) VALUES (1);");
    }

    void init_db(AmbulanceStatus ambulance_status, bool insert_call = true) {
      if (insert_call) {
        db -> execute_params("INSERT INTO calls (id, simulation_id, call_time, priority, description, x, y) VALUES (1, $1, 0, 'Alpha', 'Test Call', 0, 2);", simulation_id);
      }
      db -> execute_params("INSERT INTO hospitals (id, simulation_id, capacity, x, y) VALUES (1, $1, 10, 0, 4);", simulation_id);
      
      std::string status = (ambulance_status == AmbulanceStatus::Available) ? "Available" : "Transporting";
      db -> execute_params(
        "INSERT INTO ambulances (id, simulation_id, status, type, station_x, station_y, current_x, current_y) VALUES ($1, $2, $3, $4, $5, $6, $7, $8);",
        1, 
        simulation_id, 
        status, 
        "BLS", 
        0, 0, 0, 0
      );
    }
};

TEST_F(HandleEventTest, CallReceivedToAmbulanceMove) {
  init_db(AmbulanceStatus::Available);

  Event e = {
    simulation_id, 
    0, 
    EventType::CallReceived, 
    1, 
    std::nullopt, 
    std::nullopt,
    std::nullopt, 
    std::nullopt  
  };
  
  std::vector<Event> next_events = handle_call_received(e, calls, ambulances, hospitals, *map, *db);

  ASSERT_FALSE(next_events.empty());
  
  EXPECT_EQ(next_events[0].event_type, EventType::AmbulanceMove);
  EXPECT_EQ(next_events[0].ambulance_id.value(), 1);
  EXPECT_EQ(next_events[0].hospital_id.value(), 1);
  EXPECT_EQ(ambulances[next_events[0].ambulance_id.value()].ambulance_status, AmbulanceStatus::Responding);
}

TEST_F(HandleEventTest, CallReceivedFailure) {
  ambulances[1].ambulance_status = AmbulanceStatus::Transporting;
  init_db(AmbulanceStatus::Transporting);

  Event e = {
    simulation_id, 
    0, 
    EventType::CallReceived, 
    1, 
    std::nullopt, 
    std::nullopt,
    std::nullopt,
    std::nullopt  
  };
  std::vector<Event> next_events = handle_call_received(e, calls, ambulances, hospitals, *map, *db);

  EXPECT_TRUE(next_events.empty());
}

TEST_F(HandleEventTest, AmbulanceArriveAtSceneToTransportStart) {
  Event e = {
    simulation_id, 
    0, 
    EventType::AmbulanceArriveAtScene, 
    1, 
    1, 
    1,
    std::nullopt, 
    std::nullopt  
  };
  std::vector<Event> next_events = handle_ambulance_arrive_at_scene(e, calls, ambulances, *db);

  ASSERT_FALSE(next_events.empty());
  EXPECT_EQ(next_events[0].event_type, EventType::TransportStart);
  EXPECT_EQ(next_events[0].time, 5); // 5 for Alpha priority
}

TEST_F(HandleEventTest, TransportStartToAmbulanceMove) {
  init_db(AmbulanceStatus::Transporting); 

  Event e = {
    simulation_id, 
    0, 
    EventType::TransportStart, 
    1, 
    1, 
    1,
    std::nullopt, 
    std::nullopt  
  };
  
  std::vector<Event> next_events = handle_transport_start(e, calls, ambulances, hospitals, *map, *db);

  ASSERT_FALSE(next_events.empty());
  EXPECT_EQ(next_events[0].event_type, EventType::AmbulanceMove);
}

TEST_F(HandleEventTest, AmbulanceArriveAtHospitalToDischargeAndMove) {
  init_db(AmbulanceStatus::Transporting);

  ambulances[1].ambulance_status = AmbulanceStatus::Transporting;
  ambulances[1].current_x = hospitals[1].x;
  ambulances[1].current_y = hospitals[1].y;
  Event e = {
    simulation_id, 
    0, 
    EventType::AmbulanceArriveAtHospital, 
    1, 
    1, 
    1,
    std::nullopt, 
    std::nullopt  
  };

  std::vector<Event> next_events = handle_ambulance_arrive_at_hospital(e, calls, ambulances, hospitals, *map, *db);

  ASSERT_EQ(next_events.size(), 2);
  EXPECT_EQ(next_events[0].event_type, EventType::PatientDischarged);
  EXPECT_EQ(next_events[1].event_type, EventType::AmbulanceMove);   
  EXPECT_EQ(ambulances[1].ambulance_status, AmbulanceStatus::Available); 
}

TEST_F(HandleEventTest, AmbulanceBackAtStation) {
  init_db(AmbulanceStatus::Available); 
  ambulances[1].ambulance_status = AmbulanceStatus::Available;
  Event e = {
    simulation_id, 
    0, 
    EventType::AmbulanceBackAtStation, 
    1, 
    1, 
    1,
    std::nullopt, 
    std::nullopt 
  };
  
  std::vector<Event> next_events = handle_ambulance_back_at_station(e, ambulances, *db);

  EXPECT_TRUE(next_events.empty());
  EXPECT_EQ(ambulances[1].ambulance_status, AmbulanceStatus::Available);
}

TEST_F(HandleEventTest, AmbulanceMove) {
  init_db(AmbulanceStatus::Available);
  ambulances[1].ambulance_status = AmbulanceStatus::Responding;
  
  ambulances[1].path = {{0, 1, CellType::Road}, {0, 2, CellType::Road}};
  
  // moving
  Event e1 = {
    simulation_id, 
    0, 
    EventType::AmbulanceMove, 
    1, 
    1, 
    1,
    std::nullopt, 
    std::nullopt  
  };
  std::vector<Event> next_events = handle_ambulance_move(e1, calls, ambulances, hospitals, *db);

  ASSERT_FALSE(next_events.empty());
  EXPECT_EQ(next_events[0].event_type, EventType::AmbulanceMove); 
  EXPECT_EQ(ambulances[1].current_x, 0);
  EXPECT_EQ(ambulances[1].current_y, 1); 
  
  // complete
  Event e2 = {
    simulation_id, 
    1, 
    EventType::AmbulanceMove, 
    1, 
    1, 
    1,
    std::nullopt, 
    std::nullopt  
  };
  std::vector<Event> final_events = handle_ambulance_move(e2, calls, ambulances, hospitals, *db);
  
  ASSERT_FALSE(final_events.empty());
  EXPECT_EQ(final_events[0].event_type, EventType::AmbulanceArriveAtScene); 
  EXPECT_EQ(ambulances[1].current_x, 0);
  EXPECT_EQ(ambulances[1].current_y, 2);
}