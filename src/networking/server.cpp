#include <iostream>

#include "server.hpp"

Server::Server(asio::io_context& io_context, 
               const int port,
               SimulationHandler &simulation_handler
              )
  : io_context(io_context),
    acceptor(io_context, tcp::endpoint(tcp::v4(), port)),
    simulation_handler(simulation_handler)
  {
    std::cout << "Server started" << '\n';
    accept_client();
  }

void Server::accept_client() {
  auto client = Session::create(io_context, simulation_handler);
  
  acceptor.async_accept(client -> socket(),
    [this, client](std::error_code ec) {
      if (!ec) {
        client -> start();
      } 

      accept_client();
    }
  );
}