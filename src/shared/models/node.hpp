#pragma once

struct Node {
  int x, y;
  int g; 
  int f; 

  bool operator > (const Node &other) const {
    return f > other.f;
  }
};
