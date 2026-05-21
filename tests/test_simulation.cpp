#include <unordered_map>
#include <memory>
#include <vector>
#include <optional>
#include <queue> // Added for std::priority_queue
#include <gtest/gtest.h>

#include "logic/simulation.hpp"
#include "logic/handle_event.hpp"

#include "models/call.hpp"
#include "models/event.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/map.hpp"

#include "db/config.hpp"
#include "db/postgres.hpp"

class HandleEventTest:public ::testing::Test {
  protected:
    int simulation_id = 1;
    std::unique_ptr<Postgres> db;
    std::unordered_map<int, Call> calls;
    std::unordered_map<int, Ambulance> ambulances;
    std::unordered_map<int, Hospital> hospitals;
    std::unique_ptr<Map> map; 
    std::unordered_set<int> pending_call_ids;
    std::unordered_set<int> expired_call_ids;
    std::priority_queue<Call, std::vector<Call>, std::greater<Call>> calls_pq;

    void SetUp() override {
      calls = {{1, {1, simulation_id, 0, CallPriority::Alpha, CallStatus::Pending, 0, 2, 0, std::nullopt}}};
      ambulances = {{1, {1, simulation_id, AmbulanceStatus::Available, AmbulanceType::BLS, 0, 0, 0, 0}}};
      hospitals = {{1, {1, simulation_id, 0, 10, 0, 4}}};

      map = std::make_unique<Map>(5, 5);
      map->deserialize("A###H....................");

      db = std::make_unique<Postgres>(get_connection_url());
      db -> run_migrations(MIGRATION_PATH);
      db -> execute("TRUNCATE simulations, calls, ambulances, hospitals, dispatches, events RESTART IDENTITY CASCADE;");
      db -> execute("INSERT INTO simulations (id) VALUES (1);");

      // reset between tests
      calls_pq = std::priority_queue<Call, std::vector<Call>, std::greater<Call>>();

      pending_call_ids = {1};
    }

    void init_db(AmbulanceStatus ambulance_status, bool insert_call = true) {
      if (insert_call) {
        db -> execute_params(
          "INSERT INTO calls (id, simulation_id, call_time, priority, status, x, y) VALUES (1, $1, 0, 'Alpha', 'Pending', 0, 2);", 
          simulation_id
        );
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
    simulation_id, 0, EventType::CallReceived, 1, 
    std::nullopt, std::nullopt, std::nullopt, std::nullopt  
  };
  
  std::vector<Event> next_events = handle_call_received(e, calls, ambulances, hospitals, calls_pq, *map, *db);

  ASSERT_FALSE(next_events.empty());

  ASSERT_EQ(next_events.size(), 2);
  EXPECT_EQ(next_events[0].event_type, EventType::CallExpired);
  EXPECT_EQ(next_events[1].event_type, EventType::AmbulanceMove);
  EXPECT_EQ(next_events[1].ambulance_id.value(), 1);
  EXPECT_EQ(next_events[1].hospital_id.value(), 1);
  EXPECT_EQ(ambulances[next_events[1].ambulance_id.value()].ambulance_status, AmbulanceStatus::Responding);
}

TEST_F(HandleEventTest, CallReceivedFailure) {
  ambulances[1].ambulance_status = AmbulanceStatus::Transporting;
  init_db(AmbulanceStatus::Transporting);

  Event e = {
    simulation_id, 0, EventType::CallReceived, 1, 
    std::nullopt, std::nullopt, std::nullopt, std::nullopt  
  };
  
  std::vector<Event> next_events = handle_call_received(e, calls, ambulances, hospitals, calls_pq, *map, *db);

  ASSERT_EQ(next_events.size(), 1);
  EXPECT_EQ(next_events[0].event_type, EventType::CallExpired);
  EXPECT_EQ(calls_pq.size(), 1); 
}

TEST_F(HandleEventTest, AmbulanceArriveAtSceneToTransportStart) {
  Event e = {
    simulation_id, 0, EventType::AmbulanceArriveAtScene, 1, 1, 1, std::nullopt, std::nullopt  
  };
  std::vector<Event> next_events = handle_ambulance_arrive_at_scene(e, calls, pending_call_ids, ambulances, *db);

  ASSERT_FALSE(next_events.empty());
  EXPECT_EQ(next_events[0].event_type, EventType::TransportStart);
  EXPECT_EQ(next_events[0].time, 2); // 2 for Alpha priority
}

TEST_F(HandleEventTest, TransportStartToAmbulanceMove) {
  init_db(AmbulanceStatus::Transporting); 

  Event e = {
    simulation_id, 0, EventType::TransportStart, 1, 1, 1, std::nullopt, std::nullopt  
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
    simulation_id, 0, EventType::AmbulanceArriveAtHospital, 1, 1, 1, std::nullopt, std::nullopt  
  };

  std::vector<Event> next_events = handle_ambulance_arrive_at_hospital(e, calls, ambulances, hospitals, calls_pq, *map, *db);

  ASSERT_EQ(next_events.size(), 2);
  EXPECT_EQ(next_events[0].event_type, EventType::PatientDischarged);
  EXPECT_EQ(next_events[1].event_type, EventType::AmbulanceMove);   
  EXPECT_EQ(ambulances[1].ambulance_status, AmbulanceStatus::Available); 
}

TEST_F(HandleEventTest, AmbulanceBackAtStation) {
  init_db(AmbulanceStatus::Available); 
  ambulances[1].ambulance_status = AmbulanceStatus::Available;
  Event e = {
    simulation_id, 0, EventType::AmbulanceBackAtStation, 1, 1, 1, std::nullopt, std::nullopt 
  };
  
  std::vector<Event> next_events = handle_ambulance_back_at_station();

  EXPECT_TRUE(next_events.empty());
  EXPECT_EQ(ambulances[1].ambulance_status, AmbulanceStatus::Available);
}

TEST_F(HandleEventTest, AmbulanceMove) {
  init_db(AmbulanceStatus::Available);

  ambulances[1].ambulance_status = AmbulanceStatus::Responding;
  calls[1].status = CallStatus::Dispatched;
  calls[1].ambulance_id = 1;  
  ambulances[1].path = {{0, 1, CellType::Road}, {0, 2, CellType::Road}};
  
  // moving
  Event e1 = {
    simulation_id, 0, EventType::AmbulanceMove, 1, 1, 1, std::nullopt, std::nullopt  
  };
  std::vector<Event> next_events = handle_ambulance_move(e1, calls, ambulances, hospitals, *db);

  ASSERT_FALSE(next_events.empty());
  EXPECT_EQ(next_events[0].event_type, EventType::AmbulanceMove); 
  EXPECT_EQ(ambulances[1].current_x, 0);
  EXPECT_EQ(ambulances[1].current_y, 1); 
  
  // complete
  Event e2 = {
    simulation_id, 1, EventType::AmbulanceMove, 1, 1, 1, std::nullopt, std::nullopt  
  };
  std::vector<Event> final_events = handle_ambulance_move(e2, calls, ambulances, hospitals, *db);
  
  ASSERT_FALSE(final_events.empty());
  EXPECT_EQ(final_events[0].event_type, EventType::AmbulanceArriveAtScene); 
  EXPECT_EQ(ambulances[1].current_x, 0);
  EXPECT_EQ(ambulances[1].current_y, 2);
}

TEST_F(HandleEventTest, CallExpiredPatientSaved) {
  calls[1].status = CallStatus::Completed;
  init_db(AmbulanceStatus::Available); 

  Event e = {
    simulation_id, 
    15, 
    EventType::CallExpired, 
    1, 
    std::nullopt, std::nullopt, std::nullopt, std::nullopt  
  };

  std::vector<Event> next_events = handle_call_expired(e, calls, ambulances, hospitals, calls_pq, pending_call_ids, expired_call_ids, *map, *db);

  EXPECT_TRUE(next_events.empty());
  EXPECT_EQ(calls[1].status, CallStatus::Completed); 
}

TEST_F(HandleEventTest, CalledExpiredPatientDiedThenNewDispatch) {
  calls[1].status = CallStatus::Dispatched;
  calls[1].ambulance_id = 1;
  
  ambulances[1].ambulance_status = AmbulanceStatus::Responding;
  ambulances[1].current_x = 0;
  ambulances[1].current_y = 1; 
  
  init_db(AmbulanceStatus::Available); 

  Call c2 = {2, simulation_id, 0, CallPriority::Alpha, CallStatus::Pending, 0, 3, 0, std::nullopt};  calls[2] = c2;
  calls_pq.push(c2);

  db -> execute_params(
    "INSERT INTO calls (id, simulation_id, call_time, priority, status, x, y) VALUES (2, $1, 0, 'Alpha', 'Pending', 0, 3);", 
    simulation_id
  );

  Event e = {
    simulation_id, 
    15, 
    EventType::CallExpired, 
    1, 
    std::nullopt, std::nullopt, std::nullopt, std::nullopt  
  };

  std::vector<Event> next_events = handle_call_expired(e, calls, ambulances, hospitals, calls_pq, pending_call_ids, expired_call_ids, *map, *db);

  EXPECT_EQ(calls[1].status, CallStatus::Expired);
  ASSERT_FALSE(next_events.empty());
  EXPECT_EQ(next_events[0].event_type, EventType::AmbulanceMove);
  EXPECT_EQ(next_events[0].call_id, 2); 
  EXPECT_EQ(ambulances[1].ambulance_status, AmbulanceStatus::Responding);
  EXPECT_EQ(calls[2].status, CallStatus::Dispatched);
  ASSERT_FALSE(expired_call_ids.empty());
  EXPECT_EQ(*expired_call_ids.begin(), 1);
}
