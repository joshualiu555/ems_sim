#pragma once

#include <string>

#include <pqxx/pqxx>
#include <nlohmann/json.hpp>

#include "models/hospital.hpp"
#include "models/ambulance.hpp"
#include "models/call.hpp"
#include "models/dispatch.hpp"
#include "models/event.hpp"

class Postgres {
  public:
    Postgres(const std::string &conn_str);

    void execute(const std::string &sql);

    template <typename... Args>
    void execute_params(const std::string &sql, Args&&... args) {
      pqxx::work txn(conn);
      txn.exec(sql, pqxx::params{std::forward<Args>(args)...});
      txn.commit();
    }
    template <typename... Args>
    int return_execute_params(const std::string& sql, Args&&... args) {
      pqxx::work txn(conn);
      pqxx::row row = txn.exec(sql, pqxx::params(std::forward<Args>(args)...)).one_row();
      txn.commit();
      return row[0].as<int>();
    }

    pqxx::result query(const std::string &sql);
    template <typename... Args>
    pqxx::result query_params(const std::string& sql, Args&&... args) {
      pqxx::work txn(conn);
      pqxx::result result = txn.exec(sql, pqxx::params{std::forward<Args>(args)...});
      txn.commit();

      return result;
    }

    std::vector<int> get_all_simulations();
    int create_simulation();
    void delete_simulation(int simulation_id);

    void run_migrations(const std::string &dir);

    int insert_hospital(const Hospital &h);
    int insert_ambulance(const Ambulance &a);
    int insert_call(const Call &c);
    void insert_dispatch(const Dispatch &d);
    void insert_event(const Event &e);
    void insert_map(int simulation_id, const std::string &layout);

    void update_call_status(const std::string &status, int id);
    void add_call_end_time(const int time, int id);
    void update_hospital(int num_patients, int id);
    void update_ambulance_status(std::string status, int id);
    void update_ambulance_location(int x, int y, int id);

    nlohmann::json get_analytics(int simulation_id);

    int save_simulation(int simulation_id);
    std::vector<int> get_saved_simulations();
    std::string get_saved_layout(int saved_id);

  private:
    pqxx::connection conn;

    bool check_if_migration_exists(const std::string &file);
};
