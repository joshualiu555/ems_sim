#include <queue>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "models/call.hpp"
#include "models/event.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"

#include "handle_event.hpp"
#include "dispatch.hpp"
#include "simulation.hpp"
#include "io/logs.hpp"
#include "io/output.hpp"
#include "util/calc.hpp"

#include "db/postgres.hpp"

Simulation::Simulation(
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
)
  : 
  ambulances(ambulances), 
  hospitals(hospitals),
  db(db)
{}

void Simulation::add_call(Call &c) {
  calls[c.id] = c; 

  Event e = {
    c.time,
    EventType::CallReceived,
    c.id,
    -1,
    -1
  };

  db.insert_call(c);

  pq.push(e);
}

std::optional<Event> Simulation::create_next_event(Event &e) {
  switch(e.event_type) {
    case EventType::CallReceived:
      return handle_call_received(e, calls, ambulances, hospitals, db);
    case EventType::AmbulanceArriveAtScene:
      return handle_ambulance_arrive_at_scene(e, calls);
    case EventType::TransportStart:
      return handle_transport_start(e, calls, hospitals);
    case EventType::AmbulanceArriveAtHospital:
      return handle_ambulance_arrive_at_hospital(e, calls, ambulances, hospitals);
    case EventType::AmbulanceBackAtStation:
      return handle_ambulance_back_at_station(e, ambulances);
    default:
        return std::nullopt;
  }
}

void Simulation::run(int current_time) {
  while (!pq.empty() && pq.top().time <= current_time) {
    Event e = pq.top();
    pq.pop();
    
    std::cout << e << '\n';
    db.insert_event(e);
    
    std::optional<Event> next_event = create_next_event(e);
    if (next_event) {
      pq.push(*next_event);
    } 
  }
}
