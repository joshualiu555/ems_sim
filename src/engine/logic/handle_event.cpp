#include "handle_event.hpp"
#include "dispatch.hpp"
#include "util/calc.hpp"
#include "io/logs.hpp"

#include "db/postgres.hpp"

int find_time_elapsed(Location a, Location b) {
  int time = find_distance(a, b) / 10;
  add_time(time);
  return time;
}

std::optional<Event> handle_call_received(
  Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db
) {
  // calculate time to reach patient
  Call call = calls[e.call_id];
  std::optional<Dispatch> dispatch = create_dispatch(call, ambulances, hospitals);
  log_call();
  if (!dispatch) return std::nullopt;
  db.insert_dispatch(*dispatch);

  int time_elapsed = find_time_elapsed(call.location, ambulances[dispatch -> ambulance_id].location);

  Event next = {
    e.time + time_elapsed,
    EventType::AmbulanceArriveAtScene,
    e.call_id,
    dispatch -> ambulance_id,
    dispatch -> hospital_id
  };

  // ambulance is now unavailable
  ambulances[dispatch -> ambulance_id].ambulance_status = AmbulanceStatus::Transporting;

  return next;
}

std::optional<Event> handle_ambulance_arrive_at_scene(Event &e, std::unordered_map<int, Call> &calls) {
  // calculate time to stay on scene
  Call call = calls[e.call_id];
  int time_elapsed;
  if (call.priority == CallPriority::Alpha || call.priority == CallPriority::Bravo) {
    time_elapsed = 5;
  } else {
    time_elapsed = 10;
  }
  add_time(time_elapsed);

  Event next = {
    e.time + time_elapsed,
    EventType::TransportStart,
    e.call_id,
    e.ambulance_id,
    e.hospital_id
  };

  return next;
}

std::optional<Event> handle_transport_start(
  Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Hospital> &hospitals
) {
  // calclulate time to reach hospital
  Call call = calls[e.call_id];
  int time_elapsed = find_time_elapsed(call.location, hospitals[e.hospital_id].location);

  Event next = {
    e.time + time_elapsed,
    EventType::AmbulanceArriveAtHospital,
    e.call_id,
    e.ambulance_id,
    e.hospital_id
  };

  return next;
}

std::optional<Event> handle_ambulance_arrive_at_hospital(
  Event &e, 
  std::unordered_map<int, Call> &calls, 
  std::unordered_map<int, Ambulance> &ambulances, 
  std::unordered_map<int, Hospital> &hospitals
) {
  log_successful_call();

  // calculate time to get back to station
  Call call = calls[e.call_id];
  int distance = find_distance(hospitals[e.hospital_id].location, ambulances[e.ambulance_id].location);
  int time_elapsed = distance / 10;
  add_time(time_elapsed);

  Event next = {
    e.time + time_elapsed,
    EventType::AmbulanceBackAtStation,
    e.call_id,
    e.ambulance_id,
    e.hospital_id
  };

  return next;
}

std::optional<Event> handle_ambulance_back_at_station(Event &e, std::unordered_map<int, Ambulance> &ambulances) {
  // ambulance is available again
  ambulances[e.ambulance_id].ambulance_status = AmbulanceStatus::Available;

  return std::nullopt;
}
