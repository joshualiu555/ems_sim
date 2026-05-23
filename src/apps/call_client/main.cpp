#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <ctime>

#include <asio.hpp>
#include <nlohmann/json.hpp>

#include "models/call.hpp"
#include "util/generate.hpp"

using asio::ip::tcp;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
  int simulation_id = -1;
  for (int i = 1; i < argc - 1; i++) {
    if (std::string(argv[i]) == "--simulation-id") {
      simulation_id = std::stoi(argv[i + 1]);
    }
  }

  Bounds b = {0, 29, 0, 29};
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> time_dist(0, 7);

  try {
    asio::io_context io_context;
    tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);

    asio::connect(socket, resolver.resolve("127.0.0.1", "8080"));
    std::cout << "Client connected" << '\n';

    if (simulation_id == -1) {
      json create_simulation = {
        {"create_simulation", true}
      };
      std::string create_request = create_simulation.dump() + '\n';
      asio::write(socket, asio::buffer(create_request));

      asio::streambuf create_buffer;
      asio::read_until(socket, create_buffer, '\n');
      std::istream create_is(&create_buffer);
      std::string create_response;
      std::getline(create_is, create_response);

      simulation_id = json::parse(create_response)["simulation_id"];
    }

    std::cout << "Started simulation" << '\n';

    int current_time = 0;

    while (true) {
        Call c = generate_call(b, gen, simulation_id);
        c.time = current_time;

        json call_json = {
          {"simulation_id", simulation_id},
          {"id", c.id},
          {"priority", c.priority},
          {"time", c.time},
          {"x", c.x},
          {"y", c.y}
        };

        int tick_ms = 1000;
        bool accepted = false;
        while (!accepted) {
            std::string call_string = call_json.dump() + '\n';
            asio::write(socket, asio::buffer(call_string));

            asio::streambuf call_buffer;
            asio::read_until(socket, call_buffer, '\n');
            std::istream is(&call_buffer);
            std::string call_response;
            std::getline(is, call_response);
            std::cout << call_response << '\n';

            auto response_json = json::parse(call_response);

            if (response_json.contains("status") &&
              response_json["status"] == "Simulation not found") {
              std::cout << "Simulation deleted" << '\n';
              return 0;
            }

            if (response_json.contains("status") &&
              response_json["status"] == "Paused") {
              std::this_thread::sleep_for(std::chrono::seconds(1));
              continue;
            }

            accepted = true;

            if (response_json.contains("tick_interval_ms")) {
              tick_ms = response_json["tick_interval_ms"].get<int>();
            }
        }

      int time_elapsed = time_dist(gen);
      current_time += time_elapsed;
      std::this_thread::sleep_for(std::chrono::milliseconds(time_elapsed * tick_ms));
    }
  } catch (const std::exception &e) {
    std::cout << "Disconnected: " << e.what() << '\n';
  }

  return 0;
}