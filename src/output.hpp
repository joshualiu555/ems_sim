#include <iostream>

#include "models/ambulance.hpp"
#include "models/call.hpp"
#include "models/event.hpp"

std::ostream &operator<<(std::ostream &os, const AmbulanceStatus &as);
std::ostream &operator<<(std::ostream &os, const AmbulanceType &at);
std::ostream &operator<<(std::ostream &os, const CallPriority &cp);
std::ostream &operator<<(std::ostream &os, const Event &e);
