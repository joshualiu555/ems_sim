#pragma once

#include <string>

#include <pqxx/pqxx>

#include "../models/hospital.hpp"
#include "../models/ambulance.hpp"
#include "../models/call.hpp"
#include "../models/dispatch.hpp"
#include "../models/event.hpp"

class Postgres {
  public:
    Postgres(const std::string &conn_str);

    void execute(const std::string &sql);
    pqxx::result query(const std::string &sql);

    void insert_hospital(const Hospital &h);
    void insert_ambulance(const Ambulance &a);
    void insert_call(const Call &c);
    void insert_dispatch(const Dispatch &d);
    void insert_event(const Event &e);

  private:
    pqxx::connection conn;
};
