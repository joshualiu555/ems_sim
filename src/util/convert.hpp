#pragma once

#include "../models/ambulance.hpp"
#include "../models/call.hpp"
#include "../models/event.hpp"

std::string to_string(const AmbulanceStatus &as);
std::string to_string(const AmbulanceType &at);
std::string to_string(const CallPriority &cp);
std::string to_string(const EventType &et);
