add_executable(call_client main.cpp)

target_link_libraries(call_client PRIVATE 
  shared
  asio_standalone
  nlohmann_json
)
