add_executable(dispatch_server main.cpp)

target_link_libraries(dispatch_server PRIVATE 
  networking
)
