#pragma once

#include <unordered_map>

#include <asio.hpp>

#include "session.hpp"

#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "db/postgres.hpp"

using asio::ip::tcp;

class Server {
public:
  Server(asio::io_context &io_context, int port, 
        std::unordered_map<int, Ambulance> &ambulances,
        std::unordered_map<int, Hospital> &hospitals,
        Postgres &db);

private:
  void accept_client();

  asio::io_context &io_context;
  tcp::acceptor acceptor;

  std::unordered_map<int, Ambulance> &ambulances;
  std::unordered_map<int, Hospital> &hospitals;
  Postgres &db;
};
