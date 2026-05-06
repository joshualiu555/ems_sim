#include <cmath>

#include "models.hpp"

double distance(Location a, Location b) {
  return std::sqrt((a.lat - b.lat) * (a.lat - b.lat) + (a.lon - b.lon) * (a.lon - b.lon));
}
