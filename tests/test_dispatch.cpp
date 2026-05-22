#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "models/call.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/map.hpp" 

#include "logic/dispatch.hpp"

class DispatchTest:public::testing::Test {
  protected:
    Call call;
    std::unordered_map<int, Ambulance> ambulances;
    std::unordered_map<int, Hospital> hospitals;
    std::unique_ptr<Map> map; 

    int simulation_id = 1;

    void SetUp() override {
      call = {1, simulation_id, 0, CallPriority::Alpha, CallStatus::Pending, 0, 0};

      ambulances = {
        {1, {1, simulation_id, AmbulanceStatus::Available, AmbulanceType::BLS, 0, 0, 0, 0}}
      };

      hospitals = {
        {1, {1, simulation_id, 0, 10, 0, 0}}
      };

      map = std::make_unique<Map>(25, 25);
      std::string layout(25 * 25, '#'); 
      map -> deserialize(layout);
    }
};

TEST_F(DispatchTest, NoAmbulanceAvailable) {
  ambulances.clear(); 
  
  std::optional<Dispatch> dispatch = create_dispatch(call, ambulances, hospitals, *map);
  
  EXPECT_FALSE(dispatch);
}

TEST_F(DispatchTest, NoHospitalAvailable) {
  hospitals.clear(); 
  
  std::optional<Dispatch> dispatch = create_dispatch(call, ambulances, hospitals, *map);
  
  EXPECT_FALSE(dispatch);
}

TEST_F(DispatchTest, ValidDispatch) {
  std::optional<Dispatch> dispatch = create_dispatch(call, ambulances, hospitals, *map);
  
  EXPECT_TRUE(dispatch);
}

TEST_F(DispatchTest, AvailableAmbulancesStatus) {
  ambulances = {
    {0, {0, simulation_id, AmbulanceStatus::Available, AmbulanceType::BLS, 0, 0, 0, 0}},
    {1, {1, simulation_id, AmbulanceStatus::Transporting, AmbulanceType::BLS, 0, 0, 0, 0}},
  };

  std::vector<Ambulance> available = find_available_ambulances(call, ambulances);

  EXPECT_EQ(available.size(), 1);
  EXPECT_EQ(available[0].id, 0);
}

TEST_F(DispatchTest, AvailableAmbulancesPriority) {
  call.priority = CallPriority::Echo;
  ambulances = {
    {0, {0, simulation_id, AmbulanceStatus::Available, AmbulanceType::BLS, 0, 0, 0, 0}},
    {1, {1, simulation_id, AmbulanceStatus::Available, AmbulanceType::ALS, 0, 0, 0, 0}},
  };

  std::vector<Ambulance> available = find_available_ambulances(call, ambulances);

  EXPECT_EQ(available.size(), 1);
  EXPECT_EQ(available[0].id, 1);
}

TEST_F(DispatchTest, AvailableHospitals) {
  hospitals = {
    {0, {0, simulation_id, 0, 10, 0, 0}},
    {1, {1, simulation_id, 10, 10, 0, 0}}
  };

  std::vector<Hospital> available = find_available_hospitals(hospitals);

  EXPECT_EQ(available.size(), 1);
  EXPECT_EQ(available[0].id, 0);
}

TEST_F(DispatchTest, ClosestAmbulance) {
  std::vector<Ambulance> ambulances = {
    {0, simulation_id, AmbulanceStatus::Available, AmbulanceType::ALS, 5, 5, 5, 5},
    {1, simulation_id, AmbulanceStatus::Available, AmbulanceType::ALS, 10, 10, 10, 10},
    {2, simulation_id, AmbulanceStatus::Available, AmbulanceType::ALS, 20, 20, 20, 20}
  };

  Ambulance result = find_closest_ambulance(call, ambulances, *map);
  EXPECT_EQ(result.id, 0);
}

TEST_F(DispatchTest, ClosestHospital) {
  std::vector<Hospital> hospitals = {
    {0, simulation_id, 0, 10, 5, 5},
    {1, simulation_id, 0, 10, 10, 10},  
    {2, simulation_id, 0, 10, 20, 20}
  };

  Hospital result = find_closest_hospital(call, hospitals, *map);
  EXPECT_EQ(result.id, 0);
}