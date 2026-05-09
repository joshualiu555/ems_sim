#pragma once

#include <string>
#include <pqxx/pqxx>

class Postgres {
public:
  Postgres(const std::string& conn_str);

  void execute(const std::string& sql);
  pqxx::result query(const std::string& sql);

private:
  pqxx::connection conn;
};
