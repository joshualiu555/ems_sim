#pragma once

#include <unordered_map>

#include <asio.hpp>

#include "session.hpp"

#include "db/postgres.hpp"
#include "logic/simulation_handler.hpp"
#include "logic/simulation.hpp"

#include "models/ambulance.hpp"
#include "models/hospital.hpp"

using asio::ip::tcp;

class Server {
  public:
    Server(asio::io_context &io_context, 
           const int port, 
           SimulationHandler &simulation_handler
          );

  private:
    void accept_client();

    asio::io_context &io_context;
    tcp::acceptor acceptor;

    SimulationHandler &simulation_handler;
};
