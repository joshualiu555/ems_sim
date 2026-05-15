#include <iostream>

#include "server.hpp"

Server::Server(asio::io_context& io_context, int port,
               std::unordered_map<int, Ambulance> &ambulances,
               std::unordered_map<int, Hospital> &hospitals,
               Postgres &db,
               Simulation &simulation
              )
  : io_context(io_context),
    acceptor(io_context, tcp::endpoint(tcp::v4(), port)),
    ambulances(ambulances),
    hospitals(hospitals),
    db(db),
    simulation(simulation)
  {
    std::cout << "Server started" << '\n';
    accept_client();
  }

void Server::accept_client() {
  auto client = Session::create(io_context, simulation);
  
  acceptor.async_accept(client -> socket(),
    [this, client](std::error_code ec) {
      if (!ec) {
        client -> start();
      } 

      accept_client();
    }
  );
}