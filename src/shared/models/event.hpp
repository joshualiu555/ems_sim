#pragma once

#include <optional>

enum class EventType {
  CallReceived,
  AmbulanceArriveAtScene,
  TransportStart,
  AmbulanceArriveAtHospital,
  AmbulanceBackAtStation,
  PatientDischarged,
  AmbulanceMove,
  CallExpired
};

struct Event {
  int simulation_id;
  
  int time;
  EventType event_type;

  int call_id;
  std::optional<int> ambulance_id; 
  std::optional<int> hospital_id;

  std::optional<int> x;
  std::optional<int> y;  

  bool operator < (const Event& e) const {
    return e.time < time;
  }
};
