add_library(engine STATIC
  logic/dispatch.cpp
  logic/handle_event.cpp
  logic/simulation.cpp
  util/convert.cpp
  util/generate.cpp
  util/utility.cpp
  io/logs.cpp
  io/output.cpp
  config/config.cpp
  db/postgres.cpp
)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(engine PUBLIC pqxx)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE engine)

target_compile_definitions(engine PUBLIC 
    MIGRATION_PATH="${CMAKE_SOURCE_DIR}/db/migrations"
)
