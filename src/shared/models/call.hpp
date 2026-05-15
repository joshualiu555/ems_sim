#pragma once

#include <string>

#include "helper.hpp"

enum class CallPriority {
  Alpha,
  Bravo,
  Charlie,
  Delta,
  Echo
};

struct Call {
  int id;
  int time;
  CallPriority priority;
  std::string description;
  Location location;

  bool operator<(const Call &c) const {
    return time < c.time;
  }
};
