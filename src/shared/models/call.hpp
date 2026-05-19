#pragma once

#include <string>
#include <optional>

enum class CallStatus {
  Pending,
  Dispatched,
  Transporting,
  Completed,
  Expired
};

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
  CallStatus status;
  int x;
  int y;

  int expiration_time;
  std::optional<int> ambulance_id = std::nullopt;

  bool operator > (const Call &other) const {
    return expiration_time > other.expiration_time;
  }
};


