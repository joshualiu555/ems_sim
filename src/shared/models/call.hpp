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
  Time time;
  CallPriority priority;
  std::string description;
  Location location;

  bool operator<(const Call &c) const {
    if (time.hour == c.time.hour) return time.minute < c.time.minute;
    return time.hour < c.time.hour;
  }
};
