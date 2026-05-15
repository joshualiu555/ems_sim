#include <iostream>

#include "models/ambulance.hpp"
#include "models/call.hpp"
#include "models/event.hpp"

std::ostream &operator<<(std::ostream& os, const AmbulanceStatus &as) {
  switch (as) {
    case AmbulanceStatus::Available: 
      return os << "Available";
    case AmbulanceStatus::Transporting: 
      return os << "Transporting";
    default:
      return os << "Unknown";
  }
}

std::ostream &operator<<(std::ostream& os, const AmbulanceType &at) {
  switch (at) {
    case AmbulanceType::ALS: 
      return os << "ALS";
    case AmbulanceType::BLS: 
      return os << "BLS";
    default:
      return os << "Unknown";
  }
}

std::ostream &operator<<(std::ostream& os, const CallPriority &cp) {
  switch (cp) {
    case CallPriority::Echo: 
      return os << "Echo";
    case CallPriority::Delta: 
      return os << "Delta";
    case CallPriority::Charlie: 
      return os << "Charlie";
    case CallPriority::Bravo: 
      return os << "Bravo";
    case CallPriority::Alpha: 
      return os << "Alpha";
    default:
      return os << "Unknown";
  }
}

std::ostream &operator<<(std::ostream &os, const Event &e) {
  switch (e.event_type) {
    case EventType::CallReceived:
      return os << "Station received call " << e.call_id << " at time " << e.time;
    case EventType::AmbulanceArriveAtScene:
      return os << "Ambulance " << e.ambulance_id << " arrived at scene" << " at time " << e.time;
    case EventType::TransportStart:
      return os << "Ambulance " << e.ambulance_id << " started transport" << " at time " << e.time;
    case EventType::AmbulanceArriveAtHospital:
      return os << "Ambulance " << e.ambulance_id << " arrived at hospital " << e.hospital_id << " at time " << e.time;
    case EventType::AmbulanceBackAtStation:
      return os << "Ambulance " << e.ambulance_id << " arrived back at station " << e.hospital_id << " at time " << e.time;
    case EventType::PatientDischarged:
      return os << "Patient discharged from hospital " << e.hospital_id << " at time " << e.time;
    default:
      return os << "Unknown"; 
  }
}
