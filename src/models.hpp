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
  Echo,
  Unknown
};

struct Call {
  int id;
  int hour;
  int minute;
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
