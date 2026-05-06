#include <vector>

#include "models.hpp"

std::optional<Dispatch> create_dispatch(Call c, std::vector<Ambulance> va, std::vector<Hospital> vh);
std::vector<Ambulance> find_available_ambulances(std::vector<Ambulance> va);
Ambulance find_closest_ambulance(Call c, std::vector<Ambulance> vh);
std::vector<Hospital> find_available_hospitals(std::vector<Hospital> va);
Hospital find_closest_hospital(Call c, std::vector<Hospital> vh);
