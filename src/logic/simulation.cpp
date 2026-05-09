#include <queue>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "../models/call.hpp"
#include "../models/event.hpp"
#include "../models/ambulance.hpp"
#include "../models/hospital.hpp"

#include "handle_event.hpp"
#include "dispatch.hpp"
#include "simulation.hpp"
#include "../io/logs.hpp"
#include "../io/output.hpp"
#include "../util/utility.hpp"

Simulation::Simulation(std::unordered_map<int, Call> &calls, std::unordered_map<int, Ambulance> &ambulances, std::unordered_map<int, Hospital> &hospitals)
  : calls(calls), ambulances(ambulances), hospitals(hospitals) 
  {}

void Simulation::init() {
  for (const auto& [id, c] : calls) {
    Event e = {
      c.time,
      EventType::CallReceived,
      c.id,
      -1,
      -1
    };

    pq.push(e);
  }
}

std::optional<Event> Simulation::create_next_event(Event &e) {
  switch(e.event_type) {
    case EventType::CallReceived:
      return handle_call_received(e, calls, ambulances, hospitals);

    case EventType::AmbulanceArriveAtScene:
      return handle_ambulance_arrive_at_scene(e, calls);

    case EventType::TransportStart:
      return handle_transport_start(e, calls, hospitals);

    case EventType::AmbulanceArriveAtHospital:
      return handle_ambulance_arrive_at_hospital(e, calls, ambulances, hospitals);

    case EventType::AmbulanceBackAtStation:
      return handle_ambulance_back_at_station(e, ambulances);
  }

  return std::nullopt;
}

void Simulation::run() {
  init();

  while (!pq.empty()) {
    Event e = pq.top();
    pq.pop();
    log_event(e);
    std::optional<Event> next_event = create_next_event(e);
    if (next_event) {
      pq.push(*next_event);
    } 
  }

  output_metrics();
}
