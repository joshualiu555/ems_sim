add_library(shared STATIC
  io/output.cpp
  util/calc.cpp
  util/convert.cpp
  util/generate.cpp
)

target_include_directories(shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
