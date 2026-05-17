#pragma once

#include <string>

enum class CallPriority {
  Alpha,
  Bravo,
  Charlie,
  Delta,
  Echo
};

struct Call {
  int id;
  int simulation_id;
  int time;
  CallPriority priority;
  std::string description;
  int x;
  int y;

  bool operator < (const Call &c) const {
    return time < c.time;
  }
};
