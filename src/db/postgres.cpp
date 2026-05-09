#include "postgres.hpp"

Postgres::Postgres(const std::string& conn)
  : conn(conn)
{}

void Postgres::execute(const std::string& sql) {
  pqxx::work txn(conn);
  txn.exec(sql);
  txn.commit();
}

pqxx::result Postgres::query(const std::string& sql) {
  pqxx::work txn(conn);
  auto result = txn.exec(sql);
  txn.commit();
  return result;
}
