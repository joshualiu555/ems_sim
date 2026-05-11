# add_subdirectory(apps)
add_subdirectory(engine)
add_subdirectory(shared)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE engine shared)
