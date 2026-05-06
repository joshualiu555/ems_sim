#pragma once

#include <string>

struct Location {
  double lat;
  double lon;
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
  CallPriority priority;
  std::string description;
  Location location;
};

// Ambulance

enum class AmbulanceStatus {
  Available,
  Transporting,
  OutOfService
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
  int capacity;
  Location location;
};

// Dispatch

struct Dispatch {
  int call_id;
  int ambulance_id;
  int hospital_id;
};
