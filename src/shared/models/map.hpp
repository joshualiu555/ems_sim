#pragma once

enum class CellType {
  Empty,
  Road,
  Hospital,
  Station
};

struct Cell {
  int x;
  int y;
  CellType cell_type;

  bool operator == (const Cell &c) const {
    return x == c.x && y == c.y;
  }
};
