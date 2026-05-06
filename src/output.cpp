#include <iostream>

#include "models.hpp"

std::ostream& operator<<(std::ostream& os, AmbulanceStatus as) {
  switch (as) {
    case AmbulanceStatus::Available: 
      return os << "Available";
    case AmbulanceStatus::Transporting: 
      return os << "Transporting";
    case AmbulanceStatus::OutOfService: 
      return os << "Out of Service";
    default: 
      return os << "Unknown";
  }
}

std::ostream& operator<<(std::ostream& os, AmbulanceType at) {
  switch (at) {
    case AmbulanceType::ALS: 
      return os << "ALS";
    case AmbulanceType::BLS: 
      return os << "BLS";
    default: 
      return os << "Unknown";
  }
}

std::ostream& operator<<(std::ostream& os, CallPriority cp) {
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
