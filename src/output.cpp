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
  }
}

std::ostream &operator<<(std::ostream& os, const AmbulanceType &at) {
  switch (at) {
    case AmbulanceType::ALS: 
      return os << "ALS";
    case AmbulanceType::BLS: 
      return os << "BLS";
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
  }
}

std::ostream &operator<<(std::ostream &os, const Event &e) {
  switch (e.event_type) {
    case EventType::CallReceived:
      return os << "Station received call " << e.call_id << " at hour " << e.time.hour << " minute " << e.time.minute;
    case EventType::AmbulanceArriveAtScene:
      return os << "Ambulance " << e.ambulance_id << " arrived at scene" << " at hour " << e.time.hour << " minute " << e.time.minute;
    case EventType::TransportStart:
      return os << "Ambulance " << e.ambulance_id << " started transport" << " at hour " << e.time.hour << " minute " << e.time.minute;
    case EventType::AmbulanceArriveAtHospital:
      return os << "Ambulance " << e.ambulance_id << " arrived at hospital " << e.hospital_id << " at hour " << e.time.hour << " minute " << e.time.minute;
    case EventType::AmbulanceBackAtStation:
      return os << "Ambulance " << e.ambulance_id << " arrived back at station " << e.hospital_id << " at hour " << e.time.hour << " minute " << e.time.minute; 
  }
}
