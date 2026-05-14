#include <iostream>
#include <vector>
#include <random>

#include <asio.hpp>
#include <nlohmann/json.hpp>

#include "models/call.hpp"
#include "util/generate.hpp"

using asio::ip::tcp;
using json = nlohmann::json;

int main() {
  Bounds b = {-100, 100, -100, 100};
  std::random_device rd;
  std::mt19937 gen(rd());

  std::vector<Call> calls;
  for (int i = 0; i < 20; i++) {
    calls.push_back(generate_call(b, gen));
  }
  sort(calls.begin(), calls.end());

  asio::io_context io_context;
  tcp::socket socket(io_context);
  tcp::resolver resolver(io_context);
  
  asio::connect(socket, resolver.resolve("127.0.0.1", "8080"));

  for (const Call &c : calls) {
    json call_json = {
      {"id", c.id},
      {"priority", c.priority},
      {"hour", c.time.hour},
      {"minute", c.time.minute},
      {"lat", c.location.lat},
      {"lon", c.location.lon}
    };

    std::string call_string = call_json.dump() + '\n';
    asio::write(socket, asio::buffer(call_string));

    asio::streambuf buffer;
    asio::read_until(socket, buffer, '\n');
    std::istream is(&buffer);
    std::string dispatch;
    std::getline(is, dispatch);

    std::cout << dispatch << '\n';

    // json response = json::parse(response_line);
    // if (response["type"] == "dispatch_assignment") {
    //   std::cout << "Assigned Ambulance ID: " << response["ambulance"]["id"] << "\n";
    // }
  }

  return 0;
}
