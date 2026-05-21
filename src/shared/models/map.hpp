#pragma once

#include <optional>

enum class CellType {
  Empty,
  Road,
  Hospital,
  Station,
  Ambulance,
  Call,
  ExpiredCall
};

struct Cell {
  int x;
  int y;
  CellType cell_type;
  std::optional<int> subtype;
};
