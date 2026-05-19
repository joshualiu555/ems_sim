#include <iostream>

#include "models/event.hpp"

std::ostream &operator<<(std::ostream &os, const Event &e) {
  switch (e.event_type) {
    case EventType::CallReceived:
      return os << "In simulation " << e.simulation_id << ", station received call " << e.call_id << " at time " << e.time;
    case EventType::AmbulanceArriveAtScene:
      return os << "In simulation " << e.simulation_id << ", arrived at scene at time " << e.time;
    case EventType::TransportStart:
      return os << "In simulation " << e.simulation_id << ", started transport at time " << e.time;
    case EventType::AmbulanceArriveAtHospital:
      return os << "In simulation " << e.simulation_id << ", arrived at hospital " << e.hospital_id.value() << " at time " << e.time;
    case EventType::AmbulanceBackAtStation:
      return os << "In simulation " << e.simulation_id << ", arrived back at station " << e.ambulance_id.value() << " at time " << e.time;
    case EventType::PatientDischarged:
      return os << "In simulation " << e.simulation_id << ", patient discharged from hospital " << e.hospital_id.value() << " at time " << e.time;
    case EventType::AmbulanceMove:
      return os << "In simulation " << e.simulation_id << ", ambulance " << e.ambulance_id.value() << " moved to (" << e.x.value() << ", " << e.y.value() << ") at time " << e.time;
    case EventType::CallExpired:
      return os << "In simulation " << e.simulation_id << ", call " << e.call_id << " expired at time " << e.time;
    default:
      return os << "Unknown"; 
  }
}
