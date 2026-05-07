#pragma once

#include <string>

// Helper structs

struct Location {
  double lat;
  double lon;
};

struct Time {
  int hour;
  int minute;
};

// Call

enum class CallPriority {
  Alpha,
  Bravo,
  Charlie,
  Delta,
  Echo
};

struct Call {
  int id;
  Time time;
  CallPriority priority;
  std::string description;
  Location location;
};

// Ambulance

enum class AmbulanceStatus {
  Available,
  Transporting
};

enum class AmbulanceType {
  ALS,
  BLS
};

struct Ambulance {
  int id;
  AmbulanceStatus ambulance_status;
  AmbulanceType ambulance_type;
  Location location;
};

// Hospital

struct Hospital {
  int id;
  int num_patients;
  int capacity;
  Location location;
};

// Dispatch

struct Dispatch {
  int call_id;
  int ambulance_id;
  int hospital_id;
};

// Events

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
