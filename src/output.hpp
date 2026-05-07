#include <iostream>

#include "models.hpp"

std::ostream &operator<<(std::ostream &os, AmbulanceStatus &as);
std::ostream &operator<<(std::ostream &os, AmbulanceType &at);
std::ostream &operator<<(std::ostream &os, CallPriority &cp);
std::ostream &operator<<(std::ostream &os, Event &e);
