#include "handle_event.hpp"
#include "dispatch.hpp"
#include "util/calc.hpp"
#include "db/postgres.hpp"

int find_time_elapsed(Location a, Location b) {
  int time = find_distance(a, b) / 5;
  return time;
}

std::vector<Event> handle_call_received(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  const std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
) 
{
  Call call = calls.at(e.call_id);
  std::optional<Dispatch> dispatch = create_dispatch(call, ambulances, hospitals);
  if (!dispatch) return {};
  
  db.insert_dispatch(*dispatch);

  int time_elapsed = find_time_elapsed(call.location, ambulances[dispatch -> ambulance_id].current_location);

  Event next = {
    e.time + time_elapsed,
    EventType::AmbulanceArriveAtScene,
    e.call_id,
    dispatch -> ambulance_id,
    dispatch -> hospital_id
  };

  ambulances[dispatch -> ambulance_id].ambulance_status = AmbulanceStatus::Transporting;
  db.update_ambulance_status("Transporting", dispatch -> ambulance_id);
  
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
    e.time + time_elapsed,
    EventType::TransportStart,
    e.call_id,
    e.ambulance_id,
    e.hospital_id
  };

  ambulances[e.ambulance_id].current_location = calls.at(e.call_id).location;
  db.update_ambulance_location(calls.at(e.call_id).location.lat, calls.at(e.call_id).location.lon, e.ambulance_id);

  return {next};
}

std::vector<Event> handle_transport_start(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  const std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
) 
{
  Call call = calls.at(e.call_id);
  int time_elapsed = find_time_elapsed(call.location, hospitals.at(e.hospital_id).location);

  Event next = {
    e.time + time_elapsed,
    EventType::AmbulanceArriveAtHospital,
    e.call_id,
    e.ambulance_id,
    e.hospital_id
  };

  return {next};
}

std::vector<Event> handle_ambulance_arrive_at_hospital(
  const Event &e, 
  const std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
) 
{
  Call call = calls.at(e.call_id);
  
  // return to station
  int distance = find_distance(hospitals.at(e.hospital_id).location, ambulances.at(e.ambulance_id).station_location);
  int time_to_station = distance / 10;

  Event back_at_station = {
    e.time + time_to_station,
    EventType::AmbulanceBackAtStation,
    e.call_id,
    e.ambulance_id,
    e.hospital_id
  };

  // patient stays at hospital
  int discharge_time = (call.priority == CallPriority::Alpha || call.priority == CallPriority::Bravo) ? 10 : 20;
  
  Event discharge_event = {
    e.time + discharge_time,
    EventType::PatientDischarged,
    e.call_id,
    -1, // no ambulance needed anymore
    e.hospital_id
  };

  ambulances.at(e.ambulance_id).current_location = hospitals.at(e.hospital_id).location;
  db.update_ambulance_location(hospitals.at(e.hospital_id).location.lat, hospitals.at(e.hospital_id).location.lon, e.ambulance_id);
  
  hospitals[e.hospital_id].num_patients++;
  db.update_hospital(hospitals[e.hospital_id].num_patients, e.hospital_id);

  return {back_at_station, discharge_event};
}

std::vector<Event> handle_ambulance_back_at_station(
  const Event &e, 
  std::unordered_map<int, Ambulance> &ambulances,
  Postgres &db
) 
{
  ambulances[e.ambulance_id].current_location = ambulances[e.ambulance_id].station_location;
  ambulances[e.ambulance_id].ambulance_status = AmbulanceStatus::Available;
  
  db.update_ambulance_status("Available", e.ambulance_id);

  return {};
}

std::vector<Event> handle_patient_discharged(
  const Event &e, 
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
) 
{
  // free up a hospital bed
  hospitals[e.hospital_id].num_patients--;
  db.update_hospital(hospitals[e.hospital_id].num_patients, e.hospital_id);

  return {}; 
}
