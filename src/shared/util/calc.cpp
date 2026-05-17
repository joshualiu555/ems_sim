#include <cmath>

double find_distance(const int a_x, const int a_y, const int b_x, const int b_y) {
  return std::sqrt((a_x - b_x) * (a_x - b_x) + (a_y - b_y) * (a_y - b_y));
}
