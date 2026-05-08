#include <gtest/gtest.h>

#include "models/call.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/helper.hpp"

#include "../src/logic/dispatch.hpp"

TEST(CreateDispatchTest, NoAmbulanceAvailable) {
  Call c = {
    1, 
    {0, 0}, 
    CallPriority::Alpha, 
    "", 
    {0, 0}
  };

  std::unordered_map<int, Ambulance> ambulances;

  std::unordered_map<int, Hospital> hospitals = {
    {1, {0, 0, 10, {0, 0}}}
  };

  std::optional<Dispatch> dispatch = create_dispatch(c, ambulances, hospitals);

  EXPECT_FALSE(dispatch);
}

TEST(CreateDispatchTest, NoHospitalAvailable) {
  Call c = {
    1, 
    {0, 0}, 
    CallPriority::Alpha, 
    "", 
    {0, 0}
  };

  std::unordered_map<int, Ambulance> ambulances = {
    {1, {1, AmbulanceStatus::Available, AmbulanceType::BLS, {0, 0}}}
  };

  std::unordered_map<int, Hospital> hospitals;

  std::optional<Dispatch> dispatch = create_dispatch(c, ambulances, hospitals);

  EXPECT_FALSE(dispatch);
}

TEST(CreateDispatchTest, ValidDispatch) {
  Call c = {
    1, 
    {0, 0}, 
    CallPriority::Alpha, 
    "", 
    {0, 0}
  };

  std::unordered_map<int, Hospital> hospitals = {
    {1, {0, 0, 10, {0, 0}}}
  };
  std::unordered_map<int, Ambulance> ambulances = {
    {1, {1, AmbulanceStatus::Available, AmbulanceType::BLS, {0, 0}}}
  };

  std::optional<Dispatch> dispatch = create_dispatch(c, ambulances, hospitals);

  EXPECT_TRUE(dispatch);
}

TEST(FindAvailableTest, AvailableAmbulancesStatus) {
  Call c = {
    1,
    {0, 0},
    CallPriority::Alpha,
    "",
    {0, 0}
  };

  std::unordered_map<int, Ambulance> ambulances = {
    {0, {0, AmbulanceStatus::Available, AmbulanceType::BLS, {0, 0}}},
    {1, {1, AmbulanceStatus::Transporting, AmbulanceType::BLS, {0, 0}}},
  };

  std::vector<Ambulance> available_ambulances = find_available_ambulances(c, ambulances);

  EXPECT_EQ(available_ambulances.size(), 1);
  EXPECT_EQ(available_ambulances[0].id, 0);
}

TEST(FindAvailableTest, AvailableAmbulancesPriority) {
  Call c = {
    1,
    {0, 0},
    CallPriority::Echo,
    "",
    {0, 0}
  };

  std::unordered_map<int, Ambulance> ambulances = {
    {0, {0, AmbulanceStatus::Available, AmbulanceType::BLS, {0, 0}}},
    {1, {1, AmbulanceStatus::Available, AmbulanceType::ALS, {0, 0}}},
  };

  std::vector<Ambulance> available_ambulances = find_available_ambulances(c, ambulances);

  EXPECT_EQ(available_ambulances.size(), 1);
  EXPECT_EQ(available_ambulances[0].id, 1);
}

TEST(FindAvailableTest, AvailableHospitals) {
  std::unordered_map<int, Hospital> hospitals = {
    {0, {0, 0, 10, {0, 0}}},
    {1, {1, 10, 10, {0, 0}}}
  };

  std::vector<Hospital> available_hospitals = find_available_hospitals(hospitals);

  EXPECT_EQ(available_hospitals.size(), 1);
  EXPECT_EQ(available_hospitals[0].id, 0);
}

TEST(FindClosestTest, ClosestAmbulance) {
  Call c = {
    1,
    Time{0, 0},
    CallPriority::Alpha,
    "",
    {0, 0}
  };

  std::vector<Ambulance> ambulances = {
    {0, AmbulanceStatus::Available, AmbulanceType::ALS, {5, 5}},
    {1, AmbulanceStatus::Available, AmbulanceType::ALS, {10, 10}},
    {2, AmbulanceStatus::Available, AmbulanceType::ALS, {20, 20}}
  };

  Ambulance result = find_closest_ambulance(c, ambulances);

  EXPECT_EQ(result.id, 0);
}

TEST(FindClosestTest, ClosestHospital) {
  Call c = {
    1,
    Time{0, 0},
    CallPriority::Alpha,
    "",
    {0, 0}
  };

  std::vector<Hospital> hospitals = {
    {0, 0, 10, {5, 5}},
    {1, 0, 10, {10, 10}},  
    {2, 0, 10, {20, 20}}
  };

  Hospital result = find_closest_hospital(c, hospitals);

  EXPECT_EQ(result.id, 0);
}
