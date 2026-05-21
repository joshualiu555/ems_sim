#include <iostream>

#include <nlohmann/json.hpp>

#include "session.hpp"
#include "logic/dispatch.hpp"

#include "models/call.hpp"

using json = nlohmann::json;

std::shared_ptr<Session> Session::create(
  asio::io_context &io_context,
  SimulationHandler &simulation_handler
) 
  {
    return std::shared_ptr<Session>(new Session(io_context, simulation_handler));
  }

Session::Session(
  asio::io_context &io_context,
  SimulationHandler &simulation_handler
) 
  : socket_(io_context), 
    simulation_handler(simulation_handler)
{}

tcp::socket& Session::socket() { 
  return socket_; 
}

void Session::start() { 
  read(); 
}

void Session::read() {
  auto self(shared_from_this());

  asio::async_read_until(
    socket_,
    read_buffer,
    '\n',
    [this, self](std::error_code ec, std::size_t) {
      if (!ec) {
        std::istream is(&read_buffer);
        std::string line;
        std::getline(is, line);

        json request = json::parse(line);
        json response;

        if (request.contains("create_simulation")) {
          int simulation_id = simulation_handler.add_simulation();

          response["simulation_id"] = simulation_id;
          response["status"] = "Simulation created";

          write(response.dump() + '\n');
          return;
        } else if (request.contains("get_all_simulations")) {
          std::vector<int> ids = simulation_handler.get_all_simulations(); 

          json response;
          response["simulation_ids"] = ids;

          write(response.dump() + '\n');
        } else if (request.contains("simulation_id")) {
          int simulation_id = request.at("simulation_id");
          Simulation *simulation = simulation_handler.get_simulation(simulation_id);

          Call c;
          c.id = request.at("id");
          c.priority = request.at("priority");
          c.time = simulation -> current_time;
          
          Cell location = simulation -> get_random_road_cell();
          c.x = location.x;
          c.y = location.y;

          simulation -> add_call(c);

          response["call_id"] = c.id;
          response["status"] = "Call added";

          write(response.dump() + '\n');
          return;
        } else if (request.contains("get_simulation")) {
          int simulation_id = request["get_simulation"];
          Simulation *simulation = simulation_handler.get_simulation(simulation_id);
          
          json response;
          response["simulation"] = json::array(); 

          if (simulation == nullptr) {
            fprintf(stderr, "get_simulation: simulation %d not found\n", simulation_id);
            json response;
            response["error"] = "simulation not found";
            write(response.dump() + '\n');
            return;
          } 
          
          for (const Cell &c : simulation -> get_current_map()) {
            response["simulation"].push_back({
              {"x", c.x},
              {"y", c.y},
              {"cell_type", static_cast<int>(c.cell_type)} 
            });
          }

          write(response.dump() + '\n');
        }
      }
    }
  );
}

void Session::write(std::string message) {
  auto self(shared_from_this());
  write_message = std::move(message);

  asio::async_write(socket_, asio::buffer(write_message),
    [this, self](std::error_code ec, std::size_t) {
      if (!ec) { 
        read();
      } 
    }
  );
}
