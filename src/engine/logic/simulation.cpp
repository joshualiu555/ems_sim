#include <iostream>

#include "db/postgres.hpp"

#include "simulation.hpp"
#include "dispatch.hpp"
#include "handle_event.hpp"
#include "io/output.hpp"

#include "util/calc.hpp"
#include "util/generate.hpp"

Simulation::Simulation(
  int id,
  Postgres &db
) : 
  id(id),
  db(db)
{}

void Simulation::init(int num_ambulances, int num_hospitals) {
  Bounds b = {-30, 30, -30, 30};
  std::random_device rd;
  std::mt19937 gen(rd());

  for (int i = 0; i < num_ambulances; i++) {
    Ambulance a = generate_ambulance(b, gen, this -> id);
    a.id = db.insert_ambulance(a);   
    this -> ambulances[a.id] = a; 
  }

  for (int i = 0; i < num_hospitals; i++) {
    Hospital h = generate_hospital(b, gen, this -> id);
    h.id = db.insert_hospital(h); 
    this -> hospitals[h.id] = h; 
  }
}

void Simulation::add_call(Call &c) {
  c.simulation_id = this -> id;

  c.id = db.insert_call(c); 

  calls[c.id] = c; 

  Event e = {
    c.simulation_id,
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
