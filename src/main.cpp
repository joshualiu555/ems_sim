#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_map>

#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "models/call.hpp"

#include "logic/dispatch.hpp"
#include "logic/simulation.hpp"
#include "util/generate.hpp"
#include "io/output.hpp"
#include "io/logs.hpp"

int main() {
  std::cout << "EMS Dispatch System Simulator" << std::endl;

  Bounds b = {-100, 100, -100, 100};
  std::random_device rd;
  std::mt19937 gen(rd());

  std::unordered_map<int, Ambulance> ambulances;
  for (int i = 0; i < 5; i++) {
    Ambulance a = generate_ambulance(b, gen);
    ambulances[a.id] = a;
  }

  std::unordered_map<int, Hospital> hospitals;
  for (int i = 0; i < 5; i++) {
    Hospital h = generate_hospital(b, gen);
    hospitals[h.id] = h;
  }

  std::unordered_map<int, Call> calls;
  for (int i = 0; i < 20; i++) {
    Call c = generate_call(b, gen);
    calls[c.id] = c;
  }

  run(calls, ambulances, hospitals);

  output_metrics();
}
