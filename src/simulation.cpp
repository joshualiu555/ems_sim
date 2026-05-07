#include <queue>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "models.hpp"
#include "utility.hpp"
#include "dispatch.hpp"
#include "simulation.hpp"
#include "logs.hpp"
#include "output.hpp"

std::priority_queue<Event> pq;
int hour = 0, minute = 0;

void init(std::unordered_map<int, Call> &calls) {
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

std::optional<Event> create_next_event(Event e, std::unordered_map<int, Call> &calls, std::unordered_map<int, Ambulance> &ambulances, std::unordered_map<int, Hospital> &hospitals) {
  Event next;
    switch(e.event_type) {
      case EventType::CallReceived: {
        // calculate time to reach patient
        Call call = calls[e.call_id];
        std::optional<Dispatch> dispatch = create_dispatch(call, ambulances, hospitals);
        log_call();
        if (!dispatch) return std::nullopt;
        int distance = find_distance(call.location, ambulances[dispatch -> ambulance_id].location);
        int time = distance / 10;
        add_time(time);

        next = {
          find_next_time(e.time, time),
          EventType::AmbulanceArriveAtScene,
          e.call_id,
          dispatch -> ambulance_id,
          dispatch -> hospital_id
        };

        // ambulance is now unavailable
        ambulances[dispatch -> ambulance_id].ambulance_status = AmbulanceStatus::Transporting;

        break;   
      }
      case EventType::AmbulanceArriveAtScene: {
        // calculate time to stay on scene
        Call call = calls[e.call_id];
        int time;
        if (call.priority == CallPriority::Alpha || call.priority == CallPriority::Bravo) {
          time = 5;
        } else {
          time = 10;
        }
        add_time(time);

        next = {
          find_next_time(e.time, time),
          EventType::TransportStart,
          e.call_id,
          e.ambulance_id,
          e.hospital_id
        };
        
        break;   
      }
      case EventType::TransportStart: {
        // calclulate time to reach hospital
        Call call = calls[e.call_id];
        int distance = find_distance(call.location, hospitals[e.hospital_id].location);
        int time = distance / 10;
        add_time(time);

        next = {
          find_next_time(e.time, time),
          EventType::AmbulanceArriveAtHospital,
          e.call_id,
          e.ambulance_id,
          e.hospital_id
        };

        break;
      }
      case EventType::AmbulanceArriveAtHospital: {
        log_successful_call();

        // calculate time to get back to station
        Call call = calls[e.call_id];
        int distance = find_distance(hospitals[e.hospital_id].location, ambulances[e.ambulance_id].location);
        int time = distance / 10;
        add_time(time);

        next = {
          find_next_time(e.time, time),
          EventType::AmbulanceBackAtStation,
          e.call_id,
          e.ambulance_id,
          e.hospital_id
        };

        break;
      }

      case EventType::AmbulanceBackAtStation: {
        // ambulance is available again
        ambulances[e.ambulance_id].ambulance_status = AmbulanceStatus::Available;

        return std::nullopt;
      }
    }

  log_event(next);

  return next;
}

void run(std::unordered_map<int, Call> &calls, std::unordered_map<int, Ambulance> &ambulances, std::unordered_map<int, Hospital> &hospitals) {
  init(calls);

  while (!pq.empty()) {
    Event e = pq.top();
    std::cout << e << '\n';
    pq.pop();
    std::optional<Event> next_event = create_next_event(e, calls, ambulances, hospitals);
    if (next_event) {
      pq.push(*next_event);
    } 
  }
}
