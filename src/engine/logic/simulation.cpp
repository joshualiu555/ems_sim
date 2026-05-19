#include <iostream>
#include <vector>

#include "db/postgres.hpp"

#include "simulation.hpp"
#include "map.hpp"
#include "dispatch.hpp"
#include "handle_event.hpp"
#include "io/output.hpp"
#include "util/calc.hpp"
#include "util/generate.hpp"

#include "models/node.hpp"

Simulation::Simulation(
  int id,
  Postgres &db
) : 
  id(id),
  db(db)
{}

// builds the graph using a Prim-like algorithm
void Simulation::init(int width, int height, int num_ambulances, int num_hospitals) {
  std::random_device rd;
  std::mt19937 gen(rd());

  map = std::make_unique<Map>(width, height);
  map -> init_map();
  
  for (int i = 0; i < num_ambulances; i++) {
    // Notice the arrow instead of the dot!
    Cell c = map -> place_medical_cells(CellType::Station, gen);
    
    Ambulance a;
    a.id = 0;
    a.simulation_id = this -> id;
    a.ambulance_status = AmbulanceStatus::Available;
    a.ambulance_type = (gen() % 2 == 0) ? AmbulanceType::BLS : AmbulanceType::ALS;
    a.station_x = c.x;
    a.station_y = c.y;
    a.current_x = c.x;
    a.current_y = c.y;

    a.id = db.insert_ambulance(a);   
    this -> ambulances[a.id] = a; 
  }

  for (int i = 0; i < num_hospitals; i++) {
    Cell c = map -> place_medical_cells(CellType::Hospital, gen);
    
    Hospital h;
    h.id = 0; 
    h.simulation_id = this -> id;
    h.num_patients = 0;
    h.capacity = 10;
    h.x = c.x;
    h.y = c.y;

    h.id = db.insert_hospital(h); 
    this -> hospitals[h.id] = h; 
  }

  map -> create_main_roads();
  map -> create_side_roads(0.3, 2, 6, gen);

  std::string layout = map -> serialize();
  db.insert_map(this -> id, layout);

  map -> get_all_roads();

  map -> print_map();
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

  event_pq.push(e);
}

std::vector<Event> Simulation::create_next_event(const Event &e) {
  switch(e.event_type) {
    case EventType::CallReceived:
      return handle_call_received(e, calls, ambulances, hospitals, calls_pq, *map, db);
    case EventType::AmbulanceArriveAtScene:
      return handle_ambulance_arrive_at_scene(e, calls, ambulances, db);
    case EventType::TransportStart:
      return handle_transport_start(e, calls, ambulances, hospitals, *map, db);
    case EventType::AmbulanceArriveAtHospital:
      return handle_ambulance_arrive_at_hospital(e, calls, ambulances, hospitals, calls_pq, *map, db);
    case EventType::AmbulanceBackAtStation:
      return handle_ambulance_back_at_station();
    case EventType::PatientDischarged:
      return handle_patient_discharged(e, hospitals, db);
    case EventType::AmbulanceMove:
      return handle_ambulance_move(e, calls, ambulances, hospitals, db);
    case EventType::CallExpired:
      return handle_call_expired(e, calls, ambulances, hospitals, calls_pq, *map, db);
    default:
      return {}; 
  }
}

void Simulation::run(int current_time) {
  while (!event_pq.empty() && event_pq.top().time <= current_time) {
    Event e = event_pq.top();
    event_pq.pop();
    
    std::cout << e << '\n';
    db.insert_event(e);
    
    std::vector<Event> next_events = create_next_event(e);
    
    for (const Event& next : next_events) {
      event_pq.push(next);
    } 
  }
}

Cell Simulation::get_random_road_cell() {
  std::random_device rd;
  std::mt19937 gen(rd());
  
  return map -> get_random_road_cell(gen);
}
