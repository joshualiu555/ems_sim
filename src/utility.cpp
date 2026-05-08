#include <cmath>

#include "models/helper.hpp"

double find_distance(Location a, Location b) {
  return std::sqrt((a.lat - b.lat) * (a.lat - b.lat) + (a.lon - b.lon) * (a.lon - b.lon));
}

Time find_next_time(const Time &t, int minutes_to_add) {
  int total_minutes = t.hour * 60 + t.minute + minutes_to_add;

  Time next_time;
  next_time.hour = total_minutes / 60; 
  next_time.minute = total_minutes % 60;

  return next_time;
}
