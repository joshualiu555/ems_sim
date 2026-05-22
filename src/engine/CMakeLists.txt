add_library(engine STATIC
    db/config.cpp
    db/postgres.cpp
    logic/dispatch.cpp
    logic/handle_event.cpp
    logic/simulation_handler.cpp
    logic/simulation.cpp
    logic/map.cpp
)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(engine PUBLIC shared pqxx nlohmann_json)

target_compile_definitions(engine PUBLIC 
  MIGRATION_PATH="${CMAKE_SOURCE_DIR}/db/migrations"
)
