#pragma once

#include <optional>

#include "helper.hpp"

enum class EventType {
  CallReceived,
  AmbulanceArriveAtScene,
  TransportStart,
  AmbulanceArriveAtHospital,
  AmbulanceBackAtStation,
  PatientDischarged
};

struct Event {
  int time;
  EventType event_type;

  int call_id;
  std::optional<int> ambulance_id; 
  std::optional<int> hospital_id;

  bool operator<(const Event& e) const {
    return e.time < time;
  }
};
