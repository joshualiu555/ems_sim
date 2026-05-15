#include <iostream>

#include <nlohmann/json.hpp>

#include "session.hpp"
#include "logic/dispatch.hpp"

#include "models/call.hpp"

using json = nlohmann::json;

std::shared_ptr<Session> Session::create(
  asio::io_context &io_context,
  Simulation &simulation
) 
  {
    return std::shared_ptr<Session>(new Session(io_context, simulation));
  }

Session::Session(
  asio::io_context &io_context,
  Simulation &simulation
) 
  : socket_(io_context), 
    simulation(simulation)
{}

tcp::socket& Session::socket() { 
  return socket_; 
}

void Session::start() { 
  read(); 
}

void Session::read() {
  auto self(shared_from_this());

  asio::async_read_until(socket_, read_buffer, '\n',
    [this, self](std::error_code ec, std::size_t) {
      if (!ec) {
        std::istream is(&read_buffer);
        std::string line;
        std::getline(is, line);

        json request = json::parse(line);

        Call c;
        c.id = request["id"];
        c.priority = request["priority"];
        c.time = simulation.current_time;
        c.location.lat = request["lat"];
        c.location.lon = request["lon"];

        simulation.add_call(c);

        json response;
        response["call_id"] = c.id;

        write(response.dump() + "\n");
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