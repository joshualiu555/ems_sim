#include <iostream>

#include <nlohmann/json.hpp>

#include "session.hpp"
#include "logic/dispatch.hpp"

#include "models/call.hpp"

using json = nlohmann::json;

std::shared_ptr<Session> Session::create(
  asio::io_context &io_context,
  std::unordered_map<int, Ambulance> &ambulances,
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db) 
  {
    return std::shared_ptr<Session>(new Session(io_context, ambulances, hospitals, db));
  }

Session::Session(
  asio::io_context &io_context,
  std::unordered_map<int, Ambulance> &ambulances,
  std::unordered_map<int, Hospital> &hospitals,
  Postgres &db) 
  : socket_(io_context), 
    ambulances(ambulances), 
    hospitals(hospitals), 
    db(db) 
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
    [this, self](std::error_code ec, std::size_t length) {
      if (!ec) {
        std::istream is(&read_buffer);
        std::string line;
        std::getline(is, line);

        json incoming_data = json::parse(line);
        json response;

        Call c;
        c.id = incoming_data["id"];
        c.location.lat = incoming_data["lat"];
        c.location.lon = incoming_data["lon"];

        std::optional<Dispatch> dispatch = create_dispatch(c, ambulances, hospitals);

        if (dispatch) {
          response["call_id"] = dispatch -> call_id;
          response["ambulance_id"] = dispatch -> ambulance_id;
          response["hospital_id"] = dispatch -> hospital_id;
        } 

        write(response.dump() + "\n");
      }
    }
  );
}

void Session::write(std::string message) {
  auto self(shared_from_this());
  write_message = std::move(message);

  asio::async_write(socket_, asio::buffer(write_message),
    [this, self](std::error_code ec, std::size_t length) {
      if (!ec) { 
        read();
      } 
    }
  );
}