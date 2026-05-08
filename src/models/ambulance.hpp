#pragma once

#include "helper.hpp"

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
