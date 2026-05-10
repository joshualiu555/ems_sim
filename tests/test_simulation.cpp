#include <gtest/gtest.h>
#include <unordered_map>

#include "../src/logic/simulation.hpp"
#include "../src/logic/handle_event.hpp"
#include "../src/models/call.hpp"
#include "../src/models/ambulance.hpp"
#include "../src/models/hospital.hpp"

#include "../src/config/config.hpp"
#include "../src/db/postgres.hpp"

// tests full sequence of events
TEST(SimulationTest, CompleteCall) {
  std::unordered_map<int, Call> calls = {
    {1, {1, {0, 0}, CallPriority::Alpha, "", {0, 0}}}
  };

  std::unordered_map<int, Ambulance> ambulances = {
    {1, {1, AmbulanceStatus::Available, AmbulanceType::BLS, {0, 0}}}
  };

  std::unordered_map<int, Hospital> hospitals = {
    {1, {1, 0, 10, {0, 0}}}
  };

  Postgres db(get_connection_url());

  Simulation sim(calls, ambulances, hospitals, db);
  sim.run();

  EXPECT_EQ(ambulances[1].ambulance_status, AmbulanceStatus::Available);
}

// tests each intermediate step

TEST(HandleEventTest, CallReceivedToAmbulanceArriveAtScene) {
  Postgres db(get_connection_url());

  std::unordered_map<int, Call> calls = {
    {1, {1, {0, 0}, CallPriority::Alpha, "", {0, 0}}}
  };
  std::unordered_map<int, Ambulance> ambulances = {
    {1, {1, AmbulanceStatus::Available, AmbulanceType::BLS, {0, 0}}}
  };
  std::unordered_map<int, Hospital> hospitals = {
    {1, {1, 0, 10, {0, 0}}}
  };

  Event e = {{0, 0}, EventType::CallReceived, 1, -1, -1};
  auto next_event = handle_call_received(e, calls, ambulances, hospitals, db);

  ASSERT_TRUE(next_event);
  EXPECT_EQ(next_event -> event_type, EventType::AmbulanceArriveAtScene);
  EXPECT_EQ(next_event -> ambulance_id, 1);
  EXPECT_EQ(next_event -> hospital_id, 1);
  EXPECT_EQ(ambulances[next_event -> ambulance_id].ambulance_status, AmbulanceStatus::Transporting);
}

TEST(HandleEventTest, CallReceivedFailure) {
  Postgres db(get_connection_url());

  std::unordered_map<int, Call> calls = {
    {1, {1, {0, 0}, CallPriority::Alpha, "", {0, 0}}}
  };
  std::unordered_map<int, Ambulance> ambulances = {
    {1, {1, AmbulanceStatus::Transporting, AmbulanceType::BLS, {0, 0}}}
  };
  std::unordered_map<int, Hospital> hospitals = {
    {1, {1, 0, 10, {0, 0}}}
  };

  Event e = {{0, 0}, EventType::CallReceived, 1, -1, -1};
  auto next_event = handle_call_received(e, calls, ambulances, hospitals, db);

  EXPECT_FALSE(next_event);
}

TEST(HandleEventTest, AmbulanceArriveAtSceneToTransportStart) {
  std::unordered_map<int, Call> calls = {
    {1, {1, {0, 0}, CallPriority::Alpha, "", {0, 0}}}
  };

  Event e = {{0, 10}, EventType::AmbulanceArriveAtScene, 1, 1, 1};
  auto next_event = handle_ambulance_arrive_at_scene(e, calls);

  ASSERT_TRUE(next_event);
  EXPECT_EQ(next_event -> event_type, EventType::TransportStart);
  EXPECT_EQ(next_event -> time.minute, 15);
}

TEST(HandleEventTest, TransportStartToAmbulanceArriveAtHospital) {
  std::unordered_map<int, Call> calls = {
    {1, {1, {0, 0}, CallPriority::Alpha, "", {0, 0}}}
  };

  std::unordered_map<int, Hospital> hospitals = {
    {1, {1, 0, 10, {0, 0}}}
  };

  Event e = {{0, 0}, EventType::TransportStart, 1, 1, 1};
  auto next = handle_transport_start(e, calls, hospitals);

  ASSERT_TRUE(next);
  EXPECT_EQ(next->event_type, EventType::AmbulanceArriveAtHospital);
}

TEST(HandleEventTest, AmbulanceArriveAtHospitalToAmulanceBackAtStation) {
  std::unordered_map<int, Call> calls = {
    {1, {1, {0, 0}, CallPriority::Alpha, "", {0, 0}}}
  };
  std::unordered_map<int, Ambulance> ambulances = {
    {1, {1, AmbulanceStatus::Transporting, AmbulanceType::BLS, {0, 0}}}
  };
  std::unordered_map<int, Hospital> hospitals = {
    {1, {1, 0, 10, {0, 0}}}
  };

  Event e = {{0, 0}, EventType::AmbulanceArriveAtHospital, 1, 1, 1};

  auto next = handle_ambulance_arrive_at_hospital(e, calls, ambulances, hospitals);

  ASSERT_TRUE(next);
  EXPECT_EQ(next->event_type, EventType::AmbulanceBackAtStation);
}

TEST(HandleEventTest, BackAtStation) {
  std::unordered_map<int, Ambulance> ambulances = {
    {1, {1, AmbulanceStatus::Transporting, AmbulanceType::BLS, {0, 0}}}
  };

  Event e = {{0, 0}, EventType::AmbulanceBackAtStation, 1, 1, 1};
  auto next = handle_ambulance_back_at_station(e, ambulances);

  EXPECT_FALSE(next);
  EXPECT_EQ(ambulances[1].ambulance_status, AmbulanceStatus::Available);
}
