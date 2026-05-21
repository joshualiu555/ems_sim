#include <vector>
#include <iostream>
#include <climits>
#include <queue>
#include <algorithm>

#include "models/map.hpp"
#include "models/node.hpp"

#include "map.hpp"

Map::Map(int width, int height) 
: width(width), height(height) 
{}

void Map::init_map() {
  map.resize(width, std::vector<Cell>(height));
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      map[x][y] = {x, y, CellType::Empty};
    }
  }
}

Cell Map::place_medical_cells(CellType ct, std::mt19937 &gen) {
  std::uniform_int_distribution<> x(0, width - 1);
  std::uniform_int_distribution<> y(0, height - 1);
  
  Cell c;
  do {
    c.x = x(gen);
    c.y = y(gen);
  } while (map[c.x][c.y].cell_type != CellType::Empty);

  map[c.x][c.y].cell_type = ct;
  c.cell_type = ct;
  medical_cells.push_back(c);
  
  return c;
}

void Map::draw_road(Cell a, Cell b) {
  int x_dir = (a.x < b.x) ? 1 : -1;
  for (int x = a.x; x != b.x; x += x_dir) {
    if (map[x][a.y].cell_type == CellType::Empty) {
      map[x][a.y].cell_type = CellType::Road;
    }
    }
  
  int y_dir = (a.y < b.y) ? 1 : -1;
  for (int y = a.y; y != b.y; y += y_dir) {
    if (map[b.x][y].cell_type == CellType::Empty) {
      map[b.x][y].cell_type = CellType::Road;
    }
  }
}

void Map::create_main_roads() {
  std::vector<Cell> connected;
  std::vector<Cell> unconnected = medical_cells;

  connected.push_back(unconnected.back());
  unconnected.pop_back();

  while (!unconnected.empty()) {
    int min_distance = INT_MAX;
    int best_unconnected_idx = 0;
    Cell best_connected = connected[0];

    for (int i = 0; i < unconnected.size(); i++) {
      for (Cell c : connected) {
        int distance = std::abs(unconnected[i].x - c.x) + std::abs(unconnected[i].y - c.y);
        if (distance < min_distance) {
          min_distance = distance;
          best_unconnected_idx = i;
          best_connected = c;
        }
      }
    }

    draw_road(best_connected, unconnected[best_unconnected_idx]);
    connected.push_back(unconnected[best_unconnected_idx]);
    unconnected.erase(unconnected.begin() + best_unconnected_idx);
  }
}

void Map::create_side_roads(double branch_chance, int min_length, int max_length, std::mt19937 &gen) {
  std::vector<Cell> road_cells;
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (map[x][y].cell_type == CellType::Road) {
        road_cells.push_back(map[x][y]);
      }
    }
  }

  if (road_cells.empty()) return;

  int num_branches = (int)(road_cells.size() * branch_chance);
  std::uniform_int_distribution<> road_distance(0, road_cells.size() - 1);
  std::uniform_int_distribution<> road_dir(0, 3);
  std::uniform_int_distribution<> road_len(min_length, max_length);

  int dx[4] = {0, 0, -1, 1};
  int dy[4] = {-1, 1, 0, 0};

  for (int i = 0; i < num_branches; i++) {
    Cell start = road_cells[road_distance(gen)];
    int dir = road_dir(gen);
    int len = road_len(gen);

    int cx = start.x;
    int cy = start.y;

    for (int step = 0; step < len; ++step) {
      cx += dx[dir];
      cy += dy[dir];

      if (cx < 0 || cx >= width || cy < 0 || cy >= height || 
          map[cx][cy].cell_type == CellType::Hospital || map[cx][cy].cell_type == CellType::Station) {
            break;
          }

      map[cx][cy].cell_type = CellType::Road;
    }
  }
}

std::string Map::serialize() {
  std::string layout_str;

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (map[x][y].cell_type == CellType::Empty) layout_str += '.';
      else if (map[x][y].cell_type == CellType::Road) layout_str += '#';
      else if (map[x][y].cell_type == CellType::Hospital) layout_str += 'H';
      else if (map[x][y].cell_type == CellType::Station) layout_str += 'A';
    }
  }
  return layout_str;
}

void Map::deserialize(const std::string& layout) {
  init_map(); 

  int index = 0;
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (index >= layout.length()) return;

      char c = layout[index++];
      if (c == '.') map[x][y].cell_type = CellType::Empty;
      else if (c == '#') map[x][y].cell_type = CellType::Road;
      else if (c == 'H') map[x][y].cell_type = CellType::Hospital;
      else if (c == 'A') map[x][y].cell_type = CellType::Station;
    }
  }

  get_all_roads();
}

void Map::get_all_roads() {
  all_roads.clear();
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (map[x][y].cell_type == CellType::Road) {
        all_roads.push_back(map[x][y]);
      }
    }
  }
}

Cell Map::get_random_road_cell(std::mt19937 &gen) {  
  std::uniform_int_distribution<> dist(0, all_roads.size() - 1);
  return all_roads[dist(gen)];
}

std::vector<Cell> Map::get_static_map() {
  std::vector<Cell> static_map;

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (map[x][y].cell_type == CellType::Road || map[x][y].cell_type == CellType::Station || map[x][y].cell_type == CellType::Hospital) {
        static_map.push_back(map[x][y]);
      }
    }
  }

  return static_map;
}

std::vector<Cell> Map::find_path(Cell start, Cell end) {
  if (start.x == end.x && start.y == end.y) return {};

  std::vector<std::vector<int>> g(width, std::vector<int>(height, INT_MAX));
  std::vector<std::vector<Cell>> parent(width, std::vector<Cell>(height, {-1, -1, CellType::Empty}));

  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

  g[start.x][start.y] = 0;
  int h_start = std::abs(start.x - end.x) + std::abs(start.y - end.y);
  pq.push({start.x, start.y, 0, h_start});

  int dx[4] = {0, 0, -1, 1};
  int dy[4] = {-1, 1, 0, 0};

  bool found = false;
  while (!pq.empty()) {
    Node current = pq.top();
    pq.pop();

    if (current.x == end.x && current.y == end.y) {
      found = true;
      break;
    }

    if (current.g > g[current.x][current.y]) continue;

    for (int i = 0; i < 4; i++) {
      int nx = current.x + dx[i];
      int ny = current.y + dy[i];

      if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
      if (map[nx][ny].cell_type == CellType::Empty) {
        continue;
      }

      int next_g = current.g + 1; 

      if (next_g < g[nx][ny]) {
        g[nx][ny] = next_g;
        parent[nx][ny] = map[current.x][current.y];
        
        int h = std::abs(nx - end.x) + std::abs(ny - end.y); 
        pq.push({nx, ny, next_g, next_g + h});
      }
    }
  }

  std::vector<Cell> path;
  Cell curr = map[end.x][end.y];
  while (!(curr.x == start.x && curr.y == start.y)) {
    path.push_back(curr);
    curr = parent[curr.x][curr.y];
  }
  
  // right now, it's backwards, so reverse
  std::reverse(path.begin(), path.end());
  return path;
}
