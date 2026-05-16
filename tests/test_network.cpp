#include <string>

#include <asio.hpp>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "db/postgres.hpp"
#include "config/config.hpp"

#include "server.hpp"

#include "logic/simulation.hpp"

#include "models/call.hpp"
#include "models/ambulance.hpp"
#include "models/hospital.hpp"

using asio::ip::tcp;
using json = nlohmann::json;

Call parse_json(const std::string& payload, int server_current_time) {
  json request = json::parse(payload);
  
  Call c;
  c.id = request.at("id"); 
  c.priority = request.at("priority");
  c.time = server_current_time; 
  c.location.lat = request.at("lat");
  c.location.lon = request.at("lon");
  
  return c;
}

TEST(NetworkParsingTest, ValidJson) {
  std::string valid_payload = R"({"id": 1, "priority": 1, "time": 999, "lat": 0, "lon": 0})";
  int server_current_time = 10;

  Call c = parse_json(valid_payload, server_current_time);

  EXPECT_EQ(c.id, 1);
  // prove the master server clock overwrites the client's "999"
  EXPECT_EQ(c.time, 10); 
  EXPECT_DOUBLE_EQ(c.location.lat, 0);
  EXPECT_DOUBLE_EQ(c.location.lon, 0);
}

TEST(NetworkParsingTest, MissingRequiredFields) {
  // missing lat field
  std::string payload = R"({"id": 1, "time": 10, "priority": 1, "lon": 0})";

  EXPECT_THROW({
    parse_json(payload, 10);
  }, nlohmann::json::out_of_range);
}

TEST(NetworkIntegrationTest, ClientServerEcho) {
  std::unordered_map<int, Ambulance> ambulances;
  std::unordered_map<int, Hospital> hospitals;
  
  Postgres db(get_connection_url());
  db.run_migrations(MIGRATION_PATH);
  db.execute("TRUNCATE calls, ambulances, hospitals, dispatches, events RESTART IDENTITY CASCADE;");

  Simulation sim(ambulances, hospitals, db);

  // start server in background thread on different port
  asio::io_context server_io;
  Server server(server_io, 8000, sim); 
  std::thread server_thread([&]() { 
    server_io.run(); 
  });

  // give os time to set up server - very hacky 
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // start client on main thread
  asio::io_context client_io;
  tcp::socket socket(client_io);
  socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), 8000));

  std::string payload = R"({"id": 1, "time": 10, "priority": 1, "lat": 0, "lon": 0})";
  payload += '\n';
  asio::write(socket, asio::buffer(payload));

  asio::streambuf buffer;
  asio::read_until(socket, buffer, '\n');
  std::istream is(&buffer);
  std::string response;
  std::getline(is, response);

  json response_json = json::parse(response);
  EXPECT_EQ(response_json["call_id"], 1);

  server_io.stop();
  server_thread.join();
}
