#include "handle_event.hpp"

#include "dispatch.hpp"
#include "util/calc.hpp"

#include "db/postgres.hpp"

std::vector<Event> handle_call_received(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  const std::unordered_map<int, Hospital> &hospitals,
  Map &map, 
  Postgres &db
) 
{
  Call call = calls.at(e.call_id);
  
  std::optional<Dispatch> dispatch = create_dispatch(call, ambulances, hospitals);
  
  // right now, the call fails if no one is available - will change later
  if (!dispatch) return {};
  
  db.insert_dispatch(*dispatch);

  int ambulance_id = dispatch->ambulance_id;
  Ambulance &ambulance = ambulances[ambulance_id];

  ambulance.ambulance_status = AmbulanceStatus::Responding;
  db.update_ambulance_status("Responding", ambulance.id);
  
  // clear old paths in case this was mid drive
  ambulance.path.clear();

  Cell start = {ambulance.current_x, ambulance.current_y, CellType::Road}; 
  Cell end = {call.x, call.y, CellType::Empty};
  ambulance.path = map.find_path(start, end);

  // spawned on the same cell
  if (ambulance.path.empty()) {
    Event next = {
      e.simulation_id,
      e.time + 1,
      EventType::AmbulanceArriveAtScene,
      e.call_id,
      ambulance.id,
      dispatch -> hospital_id,
      std::nullopt, 
      std::nullopt  
    };
    return {next};
  }

  Event next = {
    e.simulation_id,
    e.time + 1,
    EventType::AmbulanceMove,
    e.call_id,
    ambulance.id,
    dispatch -> hospital_id,
    ambulance.path.front().x, 
    ambulance.path.front().y  
  };

  return {next}; 
}

std::vector<Event> handle_ambulance_arrive_at_scene(
  const Event &e, 
  const std::unordered_map<int, Call> &calls,
  std::unordered_map<int, Ambulance> &ambulances, 
  Postgres &db
) 
{
  Call call = calls.at(e.call_id);
  int time_elapsed = (call.priority == CallPriority::Alpha || call.priority == CallPriority::Bravo) ? 5 : 10;

  Event next = {
    e.simulation_id,
    e.time + time_elapsed,
    EventType::TransportStart,
    e.call_id,
    e.ambulance_id,
    e.hospital_id,
    std::nullopt, 
    std::nullopt 
  };

  ambulances[e.ambulance_id.value()].current_x = calls.at(e.call_id).x;
  ambulances[e.ambulance_id.value()].current_y = calls.at(e.call_id).y;
  db.update_ambulance_location(calls.at(e.call_id).x, calls.at(e.call_id).y, e.ambulance_id.value());

  return {next};
}

std::vector<Event> handle_transport_start(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  const std::unordered_map<int, Hospital> &hospitals,
  Map &map, 
  Postgres &db
) 
{
  int ambulance_id = e.ambulance_id.value();
  int hospital_id = e.hospital_id.value();
  Ambulance &ambulance = ambulances[ambulance_id];
  const Hospital &hospital = hospitals.at(hospital_id);

  ambulance.ambulance_status = AmbulanceStatus::Transporting;
  db.update_ambulance_status("Transporting", ambulance.id);

  ambulance.path.clear();
  Cell start = {ambulance.current_x, ambulance.current_y, CellType::Road};
  Cell end = {hospital.x, hospital.y, CellType::Hospital};
  ambulance.path = map.find_path(start, end);

  // spawned on same cell
  if (ambulance.path.empty()) {
    Event next = {
      e.simulation_id, 
      e.time + 1, 
      EventType::AmbulanceArriveAtHospital, 
      e.call_id, 
      ambulance.id, 
      hospital.id,
      std::nullopt, 
      std::nullopt  
    };
    return {next};
  }

  Event next = {
    e.simulation_id, 
    e.time + 1, 
    EventType::AmbulanceMove, 
    e.call_id, 
    ambulance.id, 
    hospital.id,
    ambulance.path.front().x, 
    ambulance.path.front().y  
  };
  return {next};
}

std::vector<Event> handle_ambulance_arrive_at_hospital(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  Map &map, 
  Postgres &db
) 
{
  int ambulance_id = e.ambulance_id.value();
  int hospital_id = e.hospital_id.value();
  Ambulance &ambulance = ambulances[ambulance_id];
  Call call = calls.at(e.call_id);
  
  hospitals[hospital_id].num_patients++;
  db.update_hospital(hospitals[hospital_id].num_patients, hospital_id);

  int discharge_time = (call.priority == CallPriority::Alpha || call.priority == CallPriority::Bravo) ? 10 : 20;
  Event discharge = {
    e.simulation_id, 
    e.time + discharge_time, 
    EventType::PatientDischarged, 
    e.call_id, 
    std::nullopt, 
    hospital_id,
    std::nullopt, 
    std::nullopt
  };

  // make ambulance available so it can now receive another call before arriving at the station
  ambulance.ambulance_status = AmbulanceStatus::Available;
  db.update_ambulance_status("Available", ambulance.id);

  ambulance.path.clear();
  Cell start = {ambulance.current_x, ambulance.current_y, CellType::Road}; 
  Cell end = {ambulance.station_x, ambulance.station_y, CellType::Station};
  ambulance.path = map.find_path(start, end);

  Event drive_home = {
    e.simulation_id, 
    e.time + 1, 
    EventType::AmbulanceMove, 
    e.call_id, 
    ambulance.id, 
    std::nullopt,
    ambulance.path.front().x, 
    ambulance.path.front().y  
  };
  return {discharge, drive_home};
}

std::vector<Event> handle_ambulance_back_at_station(
  const Event &e, 
  std::unordered_map<int, Ambulance> &ambulances,
  Postgres &db
) 
{
  ambulances[e.ambulance_id.value()].current_x = ambulances[e.ambulance_id.value()].station_x;
  ambulances[e.ambulance_id.value()].current_y = ambulances[e.ambulance_id.value()].station_y;
  ambulances[e.ambulance_id.value()].ambulance_status = AmbulanceStatus::Available;
  
  db.update_ambulance_status("Available", e.ambulance_id.value());

  return {};
}

std::vector<Event> handle_patient_discharged(
  const Event &e, 
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
) 
{
  
  // free up a hospital bed
  hospitals[e.hospital_id.value()].num_patients--;
  db.update_hospital(hospitals[e.hospital_id.value()].num_patients, e.hospital_id.value());

  return {}; 
}

std::vector<Event> handle_ambulance_move(
  const Event &e, 
  std::unordered_map<int, Call> &calls,
  std::unordered_map<int, Ambulance> &ambulances,
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
) {
  int ambulance_id = e.ambulance_id.value();
  Ambulance &ambulance = ambulances[ambulance_id];

  Cell next_step = ambulance.path.front();
  ambulance.path.erase(ambulance.path.begin());

  ambulance.current_x = next_step.x;
  ambulance.current_y = next_step.y;

  db.update_ambulance_location(ambulance.current_x, ambulance.current_y, ambulance.id);

  std::vector<Event> next_events;

  // arrived at path
  if (ambulance.path.empty()) {
    // if responding, the next event should be ambulance arrive at scene
    if (ambulance.ambulance_status == AmbulanceStatus::Responding) { 
      Event arrival = {
        e.simulation_id, 
        e.time + 1, 
        EventType::AmbulanceArriveAtScene, 
        e.call_id, 
        ambulance.id, 
        e.hospital_id,
        std::nullopt, 
        std::nullopt  
      };
      next_events.push_back(arrival);
    } 
    // if transporting, the next event should be ambulance arrive at hospital
    else if (ambulance.ambulance_status == AmbulanceStatus::Transporting) {
      Event arrival = {
        e.simulation_id, 
        e.time + 1, 
        EventType::AmbulanceArriveAtHospital, 
        e.call_id, 
        ambulance.id, 
        e.hospital_id,
        std::nullopt,
        std::nullopt 
      };
      next_events.push_back(arrival);
    }
    else if (ambulance.ambulance_status == AmbulanceStatus::Available) {
      Event arrival = {
        e.simulation_id, 
        e.time + 1, 
        EventType::AmbulanceBackAtStation, 
        e.call_id, 
        ambulance.id, 
        std::nullopt,
        std::nullopt,
        std::nullopt  
      };
      next_events.push_back(arrival);
    }
  } 
  // not arrived at path
  else {
    Event next_step_event = {
      e.simulation_id, 
      e.time + 1, 
      EventType::AmbulanceMove, 
      e.call_id, 
      ambulance.id, 
      e.hospital_id,
      ambulance.path.front().x,
      ambulance.path.front().y  
    };
    next_events.push_back(next_step_event);
  }

  return next_events;
}