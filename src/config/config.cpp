#include <cstdlib>
#include <stdexcept>

#include "config.hpp"

std::string get_connection_url() {
  const char* url = std::getenv("POSTGRES_DB_CONNECTION_URL");
  if (!url) {
    throw std::runtime_error("POSTGRES_DB_CONNECTION_URL is not set!");
  }
  return std::string(url);
}
