#include "handle_event.hpp"
#include "dispatch.hpp"

#include <iostream>

// called whenever a state makes a new dispatch possible
std::vector<Event> process_calls
(
  int simulation_id,
  int current_time,
  std::unordered_map<int, Call> &calls,
  std::unordered_map<int, Ambulance> &ambulances,
  std::unordered_map<int, Hospital> &hospitals,
  std::priority_queue<Call, std::vector<Call>, std::greater<Call>> &calls_pq,
  Map &map,
  Postgres &db
) 
{  
  std::vector<Event> next_events;
  std::vector<Call> still_waiting;

  while (!calls_pq.empty()) {
    // original state
    Call old_call = calls_pq.top();
    calls_pq.pop();

    // current state
    Call &actual_call = calls.at(old_call.id);

    // died or already handled
    if (actual_call.status != CallStatus::Pending) continue;

    std::optional<Dispatch> dispatch = create_dispatch(actual_call, ambulances, hospitals, map);
    if (!dispatch) {
      still_waiting.push_back(old_call);
      continue; 
    }

    db.insert_dispatch(*dispatch);
    actual_call.status = CallStatus::Dispatched;
    actual_call.ambulance_id = dispatch -> ambulance_id;
    actual_call.hospital_id = dispatch -> hospital_id;
    db.update_call_status("Dispatched", actual_call.id);

    Ambulance &ambulance = ambulances[dispatch -> ambulance_id];
    ambulance.ambulance_status = AmbulanceStatus::Responding;
    db.update_ambulance_status("Responding", ambulance.id);
    
    ambulance.path.clear();
    Cell start = {ambulance.current_x, ambulance.current_y, CellType::Road}; 
    Cell end = {actual_call.x, actual_call.y, CellType::Empty};
    ambulance.path = map.find_path(start, end);

    // spawned on call
    if (ambulance.path.empty()) {
      next_events.push_back({simulation_id, current_time + 1, EventType::AmbulanceArriveAtScene, actual_call.id, ambulance.id, dispatch -> hospital_id, std::nullopt, std::nullopt});
    } 
    // move to call
    else {
      next_events.push_back({simulation_id, current_time + 1, EventType::AmbulanceMove, actual_call.id, ambulance.id, dispatch -> hospital_id, ambulance.path.front().x, ambulance.path.front().y});
    }
  }

  for (const auto &sw : still_waiting) {
    calls_pq.push(sw);
  }

  return next_events;
}

std::vector<Event> handle_call_received(
  const Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  std::priority_queue<Call, std::vector<Call>, std::greater<Call>> &calls_pq,
  Map &map, 
  Postgres &db
) 
{
  Call &call = calls.at(e.call_id);
  call.status = CallStatus::Pending;
  
  // time to live (literally)
  int ttl;
  switch (call.priority) {
    case CallPriority::Alpha:   
      ttl = 50; 
      break;
    case CallPriority::Bravo:   
      ttl = 40; 
      break;
    case CallPriority::Charlie: 
      ttl = 30; 
      break;
    case CallPriority::Delta:   
      ttl = 20;  
      break;
    case CallPriority::Echo:    
      ttl = 10;  
      break;
  }

  call.expiration_time = e.time + ttl;

  calls_pq.push(call);

  // patient dies
  Event expiration_event = {
    e.simulation_id,
    call.expiration_time,
    EventType::CallExpired,
    e.call_id,
    std::nullopt, 
    std::nullopt, 
    std::nullopt, 
    std::nullopt
  };

  std::vector<Event> next_events = {expiration_event};

  std::vector<Event> dispatch_events = process_calls(e.simulation_id, e.time, calls, ambulances, hospitals, calls_pq, map, db);
  next_events.insert(next_events.end(), dispatch_events.begin(), dispatch_events.end());

  return next_events;
}

std::vector<Event> handle_ambulance_arrive_at_scene(
  const Event &e, 
  std::unordered_map<int, Call> &calls,
  std::unordered_set<int> &pending_call_ids,
  std::unordered_map<int, Ambulance> &ambulances, 
  Postgres &db
) 
{
  Call &call = calls.at(e.call_id);
  pending_call_ids.erase(call.id);
  
  int remaining_time = call.expiration_time - e.time;
  int new_expiration_time = e.time + (remaining_time * 2);
  
  call.expiration_time = new_expiration_time;

  Event expiration = {
    e.simulation_id,
    new_expiration_time,
    EventType::CallExpired,
    e.call_id,
    std::nullopt, 
    std::nullopt, 
    std::nullopt, 
    std::nullopt 
  };

  int time_elapsed = (call.priority == CallPriority::Alpha || call.priority == CallPriority::Bravo) ? 2 : 5;
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

  ambulances[e.ambulance_id.value()].current_x = call.x;
  ambulances[e.ambulance_id.value()].current_y = call.y;
  db.update_ambulance_location(call.x, call.y, e.ambulance_id.value());

  return {next, expiration};
}

std::vector<Event> handle_transport_start(
  const Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  const std::unordered_map<int, Hospital> &hospitals,
  Map &map, 
  Postgres &db
) 
{
  Call &call = calls.at(e.call_id);
  if (call.status == CallStatus::Expired) {
    return {};
  }

  int ambulance_id = e.ambulance_id.value();
  int hospital_id = e.hospital_id.value();
  Ambulance &ambulance = ambulances[ambulance_id];
  const Hospital &hospital = hospitals.at(hospital_id);

  calls.at(e.call_id).status = CallStatus::Transporting;
  db.update_call_status("Transporting", e.call_id); 

  ambulance.ambulance_status = AmbulanceStatus::Transporting;
  db.update_ambulance_status("Transporting", ambulance.id);

  ambulance.path.clear();
  Cell start = {ambulance.current_x, ambulance.current_y, CellType::Road};
  Cell end = {hospital.x, hospital.y, CellType::Hospital};
  ambulance.path = map.find_path(start, end);

  if (ambulance.path.empty()) {
    Event next = {e.simulation_id, e.time + 1, EventType::AmbulanceArriveAtHospital, e.call_id, ambulance.id, hospital.id, std::nullopt, std::nullopt};
    return {next};
  }

  Event next = {e.simulation_id, e.time + 1, EventType::AmbulanceMove, e.call_id, ambulance.id, hospital.id, ambulance.path.front().x, ambulance.path.front().y};
  return {next};
}

std::vector<Event> handle_ambulance_arrive_at_hospital(
  const Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  std::priority_queue<Call, std::vector<Call>, std::greater<Call>> &calls_pq,
  Map &map, 
  Postgres &db
) 
{
  int ambulance_id = e.ambulance_id.value();
  int hospital_id = e.hospital_id.value();
  Ambulance &ambulance = ambulances[ambulance_id];
  const Call &call = calls.at(e.call_id);
  
  calls.at(e.call_id).status = CallStatus::Completed;
  db.update_call_status("Completed", e.call_id); 
  db.add_call_end_time(e.time, call.id);

  hospitals[hospital_id].num_patients++;
  db.update_hospital(hospitals[hospital_id].num_patients, hospital_id);

  int discharge_time = (call.priority == CallPriority::Alpha || call.priority == CallPriority::Bravo) ? 3 : 7;
  Event discharge = {e.simulation_id, e.time + discharge_time, EventType::PatientDischarged, e.call_id, std::nullopt, hospital_id, std::nullopt, std::nullopt};

  ambulance.ambulance_status = AmbulanceStatus::Available;
  db.update_ambulance_status("Available", ambulance.id);

  ambulance.path.clear();
  Cell start = {ambulance.current_x, ambulance.current_y, CellType::Road}; 
  CellType ct = (ambulance.ambulance_type == AmbulanceType::ALS) ? CellType::ALSStation : CellType::BLSStation;
  Cell end = {ambulance.station_x, ambulance.station_y, ct};
  ambulance.path = map.find_path(start, end);

  std::vector<Event> events = {discharge};
  std::vector<Event> dispatch_events = process_calls(e.simulation_id, e.time, calls, ambulances, hospitals, calls_pq, map, db);
  if (!dispatch_events.empty()) {
    events.insert(events.end(), dispatch_events.begin(), dispatch_events.end());
  } else {
    if (ambulance.path.empty()) {
      events.push_back({e.simulation_id, e.time + 1, EventType::AmbulanceBackAtStation, e.call_id, ambulance.id, std::nullopt, std::nullopt, std::nullopt});
    } else {
      events.push_back({e.simulation_id, e.time + 1, EventType::AmbulanceMove, e.call_id, ambulance.id, std::nullopt, ambulance.path.front().x, ambulance.path.front().y});
    }
  }

  return events;
}

std::vector<Event> handle_ambulance_back_at_station() {
  return {};
}

std::vector<Event> handle_patient_discharged(
  const Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  std::unordered_set<int> &patient_discharged_ids,
  std::priority_queue<Call, std::vector<Call>, std::greater<Call>> &calls_pq,
  Map &map, 
  Postgres &db
)
{
  hospitals[e.hospital_id.value()].num_patients--;
  db.update_hospital(hospitals[e.hospital_id.value()].num_patients, e.hospital_id.value());
  patient_discharged_ids.insert(e.call_id);
  return process_calls(e.simulation_id, e.time, calls, ambulances, hospitals, calls_pq, map, db);
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
  const Call &event_call = calls.at(e.call_id);

  // the ambulance got poached in the middle of driving
  // this is a ghost move
  if (ambulance.ambulance_status == AmbulanceStatus::Responding) {
    if (event_call.status != CallStatus::Dispatched || event_call.ambulance_id != ambulance.id) {
      return {}; 
    }
  } 
  else if (ambulance.ambulance_status == AmbulanceStatus::Transporting) {
    if (event_call.status != CallStatus::Transporting || event_call.ambulance_id != ambulance.id) {
      return {}; 
    }
  }

  if (ambulance.path.empty()) return {};

  Cell next_step = ambulance.path.front();
  ambulance.path.erase(ambulance.path.begin());

  ambulance.current_x = next_step.x;
  ambulance.current_y = next_step.y;

  db.update_ambulance_location(ambulance.current_x, ambulance.current_y, ambulance.id);

  std::vector<Event> next_events;

  if (ambulance.path.empty()) {
    if (ambulance.ambulance_status == AmbulanceStatus::Responding) { 
      Event arrival = {e.simulation_id, e.time + 1, EventType::AmbulanceArriveAtScene, e.call_id, ambulance.id, e.hospital_id, std::nullopt, std::nullopt};
      next_events.push_back(arrival);
    } 
    else if (ambulance.ambulance_status == AmbulanceStatus::Transporting) {
      Event arrival = {e.simulation_id, e.time + 1, EventType::AmbulanceArriveAtHospital, e.call_id, ambulance.id, e.hospital_id, std::nullopt, std::nullopt};
      next_events.push_back(arrival);
    }
    else if (ambulance.ambulance_status == AmbulanceStatus::Available) {
      Event arrival = {e.simulation_id, e.time + 1, EventType::AmbulanceBackAtStation, e.call_id, ambulance.id, std::nullopt, std::nullopt, std::nullopt};
      next_events.push_back(arrival);
    }
  } 
  else {
    Cell upcoming_step = ambulance.path.front();
    Event next_step_event = {e.simulation_id, e.time + 1, EventType::AmbulanceMove, e.call_id, ambulance.id, e.hospital_id, upcoming_step.x, upcoming_step.y};
    next_events.push_back(next_step_event);
  }

  return next_events;
}

std::vector<Event> handle_call_expired(
  const Event &e, 
  std::unordered_map<int, Call> &calls,
  std::unordered_map<int, Ambulance> &ambulances,
  std::unordered_map<int, Hospital> &hospitals,
  std::priority_queue<Call, std::vector<Call>, std::greater<Call>> &pending_calls,
  std::unordered_set<int> &pending_call_ids,
  std::unordered_set<int> &expired_call_ids,
  Map &map,
  Postgres &db
) {
  Call &call = calls.at(e.call_id);
  if (e.time < call.expiration_time) {
    return {};
  }
  if (call.status == CallStatus::Completed || call.status == CallStatus::Transporting || call.status == CallStatus::Expired) {
    return {}; 
  }
  call.status = CallStatus::Expired;
  db.update_call_status("Expired", call.id); 
  db.add_call_end_time(e.time, call.id);
  pending_call_ids.erase(call.id);
  expired_call_ids.insert(call.id);

  if (call.ambulance_id.has_value()) {
    Ambulance &ambulance = ambulances[call.ambulance_id.value()];
    
    if (ambulance.ambulance_status == AmbulanceStatus::Responding) {
      ambulance.ambulance_status = AmbulanceStatus::Available;
      db.update_ambulance_status("Available", ambulance.id);
      ambulance.path.clear(); // <-- clear only, no station path yet

      std::vector<Event> dispatch_events = process_calls(e.simulation_id, e.time, calls, ambulances, hospitals, pending_calls, map, db);
      if (!dispatch_events.empty()) {
        return dispatch_events;
      }

      // no new call so return back to station
      CellType ct = (ambulance.ambulance_type == AmbulanceType::ALS) ? CellType::ALSStation : CellType::BLSStation;
      Cell start = {ambulance.current_x, ambulance.current_y, CellType::Road};
      Cell end = {ambulance.station_x, ambulance.station_y, ct};
      ambulance.path = map.find_path(start, end);

      if (ambulance.path.empty()) {
        return {{e.simulation_id, e.time + 1, EventType::AmbulanceBackAtStation, e.call_id, ambulance.id, std::nullopt, std::nullopt, std::nullopt}};
      } else {
        return {{e.simulation_id, e.time + 1, EventType::AmbulanceMove, e.call_id, ambulance.id, std::nullopt, ambulance.path.front().x, ambulance.path.front().y}};
      }
    }
  }

  return {};
}