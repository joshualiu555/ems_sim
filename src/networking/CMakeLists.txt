add_library(networking STATIC
  server.cpp
  session.cpp
)

target_link_libraries(networking PUBLIC 
  asio_standalone
  nlohmann_json
  engine
)

target_include_directories(networking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
