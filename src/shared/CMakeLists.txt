add_library(shared STATIC
  util/convert.cpp
  util/generate.cpp
)

target_include_directories(shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
