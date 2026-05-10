#include "../models/ambulance.hpp"
#include "../models/call.hpp"
#include "../models/event.hpp"

std::string to_string(const AmbulanceStatus &as) {
    return as == AmbulanceStatus::Available ? "Available" : "Transporting";
}

std::string to_string(const AmbulanceType &at) {
    return at == AmbulanceType::ALS ? "ALS" : "BLS";
}

std::string to_string(const CallPriority &cp) {
    switch(cp) {
      case CallPriority::Alpha: 
        return "Alpha";
      case CallPriority::Bravo: 
        return "Bravo";
      case CallPriority::Charlie: 
        return "Charlie";
      case CallPriority::Delta:  
        return "Delta";
      case CallPriority::Echo: 
        return "Echo";
      default:
        return "Unknown";
    }
}

std::string to_string(const EventType &et) {
    switch(et) {
      case EventType::CallReceived: 
        return "CallReceived";
      case EventType::AmbulanceArriveAtScene: 
        return "AmbulanceArriveAtScene";
      case EventType::TransportStart: 
        return "TransportStart";
      case EventType::AmbulanceArriveAtHospital: 
        return "AmbulanceArriveAtHospital";
      case EventType::AmbulanceBackAtStation: 
        return "AmbulanceBackAtStation";
      default:
        return "Unknown";
    }
}
