#pragma once

#include <random>
#include <vector>

#include "models/map.hpp"
#include "models/node.hpp"

class Map {
  public:
    Map(
      int width,
      int height
    );

    void init_map();
    Cell place_medical_cells(CellType ct, std::mt19937 &gen);
    // connect medical points
    void create_main_roads();
    // one-off roads
    void create_side_roads(double branch_chance, int min_length, int max_length, std::mt19937 &gen);
    void print_map();

    std::string serialize();
    void deserialize(const std::string &layout);

    void get_all_roads();
    Cell get_random_road_cell(std::mt19937 &gen);

    std::vector<Cell> find_path(Cell start, Cell end);

  private:
    int width;
    int height;
    std::vector<std::vector<Cell>> map;
    std::vector<Cell> medical_cells;

    std::vector<Cell> all_roads;

    // draw L-shaped road
    void draw_road(Cell a, Cell b);
};
