#include <iostream>
#include <vector>
#include <random>

#include "generate.hpp"
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

int main() {
  std::cout << "EMS Dispatch System Simulator" << std::endl;

  // call loop
  Bounds b = {-100, 100, -100, 100};
  std::random_device rd;
  std::mt19937 gen(rd());

  std::vector<Ambulance> ambulances;
  for (int i = 0; i < 5; i++) {
    ambulances.push_back(generate_ambulance(b, gen));
  }

  for (Ambulance a : ambulances) {
    std::cout << "Ambulance ID: " << a.id << '\n';
    std::cout << "Ambulance Status: " << a.ambulance_status << '\n';
    std::cout << "Ambulance Type: " << a.ambulance_type << '\n';
    std::cout << "Ambulance Location: " << a.location.lat << ' ' << a.location.lon << '\n';
  }
}
