#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <asio.hpp>

#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "db/postgres.hpp"

using asio::ip::tcp;

class Session:public std::enable_shared_from_this<Session> {
public:
  static std::shared_ptr<Session> create(
    asio::io_context &io_context,
    std::unordered_map<int, Ambulance> &ambulances,
    std::unordered_map<int, Hospital> &hospitals,
    Postgres &db
  );

  tcp::socket &socket();

  void start();

private:
    explicit Session(
      asio::io_context &io_context,
      std::unordered_map<int, Ambulance> &ambulances,
      std::unordered_map<int, Hospital> &hospitals,
      Postgres &db
    );

    void read();
    void write(std::string message);

    tcp::socket socket_;
    asio::streambuf read_buffer;
    std::string write_message;

    std::unordered_map<int, Ambulance> &ambulances;
    std::unordered_map<int, Hospital> &hospitals;
    Postgres &db;
};
