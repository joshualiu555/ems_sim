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
};
