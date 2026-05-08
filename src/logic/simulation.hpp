#include <vector>
#include <unordered_map>
#include <map>
#include <optional>

#include "../models/call.hpp"
#include "../models/event.hpp"
#include "../models/ambulance.hpp"
#include "../models/hospital.hpp"

void init(std::unordered_map<int, Call> &calls);
std::optional<Event> create_next_event(Event e, std::unordered_map<int, Call> &calls, std::unordered_map<int, Ambulance> &ambulances, std::unordered_map<int, Hospital> &hospitals);
void run(std::unordered_map<int, Call> &calls, std::unordered_map<int, Ambulance> &ambulances, std::unordered_map<int, Hospital> &ospitals);
