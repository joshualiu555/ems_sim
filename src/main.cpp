#include <iostream>
#include <vector>
#include <random>

#include "generate.hpp"
#include "models.hpp"
#include "output.hpp"

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

  std::vector<Hospital> hospitals;
  for (int i = 0; i < 5; i++) {
    hospitals.push_back(generate_hospital(b, gen));
  }

  std::vector<Call> calls;
  for (int i = 0; i < 10; i++) {
    calls.push_back(generate_call(b, gen));
  }

  for (Ambulance a : ambulances) {
    std::cout << "Ambulance ID: " << a.id << '\n';
    std::cout << "Ambulance Status: " << a.ambulance_status << '\n';
    std::cout << "Ambulance Type: " << a.ambulance_type << '\n';
    std::cout << "Ambulance Location: " << a.location.lat << ' ' << a.location.lon << '\n';
  }

  std::cout << '\n';

  for (Hospital h : hospitals) {
    std::cout << "Hospital ID: " << h.id << '\n';
    std::cout << "Hospital Capacity: " << h.capacity << '\n';
    std::cout << "Hospital Location: " << h.location.lat << ' ' << h.location.lon << '\n';
  }

  std::cout << '\n';

  for (Call c : calls) {
    std::cout << "Call ID: " << c.id << '\n';
    std::cout << "Hour: " << c.hour << '\n';
    std::cout << "Minute: " << c.minute << '\n';
    std::cout << "Call Priority: " << c.priority << '\n';
    std::cout << "Location: " << c.location.lat << ' ' << c.location.lon << '\n';
  }
}
