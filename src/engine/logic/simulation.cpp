#include <iostream>

#include "db/postgres.hpp"

#include "simulation.hpp"
#include "dispatch.hpp"
#include "handle_event.hpp"
#include "io/output.hpp"

#include "util/calc.hpp"

Simulation::Simulation(
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
) : ambulances(ambulances), hospitals(hospitals), db(db) 
{}

void Simulation::add_call(Call &c) {
  calls[c.id] = c; 
  
  db.insert_call(c); 

  Event e = {
    c.time,
    EventType::CallReceived,
    c.id,
    std::nullopt,
    std::nullopt
  };

  pq.push(e);
}

std::vector<Event> Simulation::create_next_event(const Event &e) {
  switch(e.event_type) {
    case EventType::CallReceived:
      return handle_call_received(e, calls, ambulances, hospitals, db);
    case EventType::AmbulanceArriveAtScene:
      return handle_ambulance_arrive_at_scene(e, calls, ambulances, db);
    case EventType::TransportStart:
      return handle_transport_start(e, calls, ambulances, hospitals, db);
    case EventType::AmbulanceArriveAtHospital:
      return handle_ambulance_arrive_at_hospital(e, calls, ambulances, hospitals, db);
    case EventType::AmbulanceBackAtStation:
      return handle_ambulance_back_at_station(e, ambulances, db);
    case EventType::PatientDischarged:
      return handle_patient_discharged(e, hospitals, db);
    default:
      return {}; 
  }
}

void Simulation::run(int current_time) {
  while (!pq.empty() && pq.top().time <= current_time) {
    Event e = pq.top();
    pq.pop();
    
    std::cout << e << '\n';
    db.insert_event(e);
    
    std::vector<Event> next_events = create_next_event(e);
    
    for (const Event& next : next_events) {
      pq.push(next);
    } 
  }
}
