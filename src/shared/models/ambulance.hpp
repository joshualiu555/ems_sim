#pragma once

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
  int simulation_id;
  AmbulanceStatus ambulance_status;
  AmbulanceType ambulance_type;
  int station_x;
  int station_y;
  int current_x;
  int current_y;
};
