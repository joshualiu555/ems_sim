#include <iostream>
#include <vector>

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
  map -> print_map();

  std::string layout = map -> serialize();
  db.insert_map(this -> id, layout);
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

Map::Map(int width, int height) 
: width(width), height(height) 
{}

void Map::init_map() {
  map.resize(width, std::vector<Cell>(height));
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      map[x][y] = {x, y, CellType::Empty};
    }
  }
}

Cell Map::place_medical_cells(CellType ct, std::mt19937 &gen) {
  std::uniform_int_distribution<> x(0, width - 1);
  std::uniform_int_distribution<> y(0, height - 1);
  
  Cell c;
  do {
    c.x = x(gen);
    c.y = y(gen);
  } while (map[c.x][c.y].cell_type != CellType::Empty);

  map[c.x][c.y].cell_type = ct;
  c.cell_type = ct;
  medical_cells.push_back(c);
  
  return c;
}

void Map::draw_road(Cell a, Cell b) {
  int x_dir = (a.x < b.x) ? 1 : -1;
  for (int x = a.x; x != b.x; x += x_dir) {
    if (map[x][a.y].cell_type == CellType::Empty) {
      map[x][a.y].cell_type = CellType::Road;
    }
    }
  
  int y_dir = (a.y < b.y) ? 1 : -1;
  for (int y = a.y; y != b.y; y += y_dir) {
    if (map[b.x][y].cell_type == CellType::Empty) {
      map[b.x][y].cell_type = CellType::Road;
    }
  }
}

void Map::create_main_roads() {
  // if there are 0 or 1 medical_cells, nothing can connect
  if (medical_cells.size() < 2) return;

  std::vector<Cell> connected;
  std::vector<Cell> unconnected = medical_cells;

  connected.push_back(unconnected.back());
  unconnected.pop_back();

  while (!unconnected.empty()) {
    int min_distance = INT_MAX;
    int best_unconnected_idx = 0;
    Cell best_connected = connected[0];

    for (int i = 0; i < unconnected.size(); i++) {
      for (Cell c : connected) {
        int distance = std::abs(unconnected[i].x - c.x) + std::abs(unconnected[i].y - c.y);
        if (distance < min_distance) {
          min_distance = distance;
          best_unconnected_idx = i;
          best_connected = c;
        }
      }
    }

    draw_road(best_connected, unconnected[best_unconnected_idx]);
    connected.push_back(unconnected[best_unconnected_idx]);
    unconnected.erase(unconnected.begin() + best_unconnected_idx);
  }
}

void Map::create_side_roads(double branch_chance, int min_length, int max_length, std::mt19937 &gen) {
  std::vector<Cell> road_cells;
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (map[x][y].cell_type == CellType::Road) {
        road_cells.push_back(map[x][y]);
      }
    }
  }

  if (road_cells.empty()) return;

  int num_branches = (int)(road_cells.size() * branch_chance);
  std::uniform_int_distribution<> road_distance(0, road_cells.size() - 1);
  std::uniform_int_distribution<> road_dir(0, 3);
  std::uniform_int_distribution<> road_len(min_length, max_length);

  int dx[4] = {0, 0, -1, 1};
  int dy[4] = {-1, 1, 0, 0};

  for (int i = 0; i < num_branches; i++) {
    Cell start = road_cells[road_distance(gen)];
    int dir = road_dir(gen);
    int len = road_len(gen);

    int cx = start.x;
    int cy = start.y;

    for (int step = 0; step < len; ++step) {
      cx += dx[dir];
      cy += dy[dir];

      if (cx < 0 || cx >= width || cy < 0 || cy >= height || 
          map[cx][cy].cell_type == CellType::Hospital || map[cx][cy].cell_type == CellType::Station) {
            break;
          }

      map[cx][cy].cell_type = CellType::Road;
    }
  }
}

void Map::print_map() {
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (map[x][y].cell_type == CellType::Empty) std::cout << '.';
      else if (map[x][y].cell_type == CellType::Road) std::cout << '#';
      else if (map[x][y].cell_type == CellType::Hospital) std::cout << 'H';
      else if (map[x][y].cell_type == CellType::Station) std::cout << 'A';
    }
    std::cout << '\n';
  }
}

std::string Map::serialize() {
  std::string layout_str;

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (map[x][y].cell_type == CellType::Empty) layout_str += '.';
      else if (map[x][y].cell_type == CellType::Road) layout_str += '#';
      else if (map[x][y].cell_type == CellType::Hospital) layout_str += 'H';
      else if (map[x][y].cell_type == CellType::Station) layout_str += 'A';
    }
  }
  return layout_str;
}

void Map::deserialize(const std::string& layout) {
  init_map(); 

  int index = 0;
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (index >= layout.length()) return;

      char c = layout[index++];
      if (c == '.') map[x][y].cell_type = CellType::Empty;
      else if (c == '#') map[x][y].cell_type = CellType::Road;
      else if (c == 'H') map[x][y].cell_type = CellType::Hospital;
      else if (c == 'A') map[x][y].cell_type = CellType::Station;
    }
  }
}
