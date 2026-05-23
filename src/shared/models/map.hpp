#pragma once

#include <optional>

enum class CellType {
  Empty,
  Road,
  Hospital,
  ALSStation,
  BLSStation,
  Ambulance,
  Call,
  PatientDischarged,
  ExpiredCall
};

struct Cell {
  int x;
  int y;
  CellType cell_type;
  std::optional<int> subtype;
};
