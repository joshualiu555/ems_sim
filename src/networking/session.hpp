#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <asio.hpp>

#include "logic/simulation_handler.hpp"
#include "logic/simulation.hpp"

#include "models/ambulance.hpp"
#include "models/hospital.hpp"
#include "db/postgres.hpp"

using asio::ip::tcp;

class Session:public std::enable_shared_from_this<Session> {
  public:
    static std::shared_ptr<Session> create(
      asio::io_context &io_context,
      SimulationHandler &simulation_handler
    );

    tcp::socket &socket();

    void start();

  private:
      Session(
        asio::io_context &io_context,
        SimulationHandler &simulation_handler
      );

      void read();
      void write(std::string message);

      tcp::socket socket_;
      asio::streambuf read_buffer;
      std::string write_message;

      SimulationHandler &simulation_handler;
};
