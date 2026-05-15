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

int main() {
  Bounds b = {-30, 30, -30, 30};
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> time_dist(1, 7); 

  asio::io_context io_context;
  tcp::socket socket(io_context);
  tcp::resolver resolver(io_context);
  
  asio::connect(socket, resolver.resolve("127.0.0.1", "8080"));
  std::cout << "Client started" << '\n';

  int current_time = 0;
  while (true) {
    Call c = generate_call(b, gen);
    c.time = current_time;

    json call_json = {
      {"id", c.id},
      {"priority", c.priority},
      {"time", c.time},
      {"lat", c.location.lat},
      {"lon", c.location.lon}
    };

    std::string call_string = call_json.dump() + '\n';
    asio::write(socket, asio::buffer(call_string));

    asio::streambuf buffer;
    asio::read_until(socket, buffer, '\n');
    std::istream is(&buffer);
    std::string response;
    std::getline(is, response);
    
    std::cout << response << '\n';

    int time_elapsed = time_dist(gen);
    current_time += time_elapsed;
    std::this_thread::sleep_for(std::chrono::seconds(time_elapsed));
  }

  return 0;
}