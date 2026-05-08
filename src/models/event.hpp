#pragma once

#include "helper.hpp"

enum class EventType {
  CallReceived,
  AmbulanceArriveAtScene,
  TransportStart,
  AmbulanceArriveAtHospital,
  AmbulanceBackAtStation
};

struct Event {
  Time time;
  EventType event_type;

  int call_id = -1;
  int ambulance_id = -1;
  int hospital_id = -1;

  bool operator<(const Event& e) const {
    if (time.hour == e.time.hour) return time.minute > e.time.minute;
    return time.hour > e.time.hour;
  }
};
