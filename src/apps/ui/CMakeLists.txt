add_executable(ui main.cpp)

target_link_libraries(ui PRIVATE
  engine        
  imgui 
  asio_standalone
  nlohmann_json
)

if (APPLE)
  target_link_libraries(ui PRIVATE 
    "-framework OpenGL" 
    "-framework Cocoa" 
    "-framework IOKit" 
    "-framework CoreVideo"
  )
elseif (UNIX)
  target_link_libraries(ui PRIVATE
    GL
    X11
    Xi
    Xrandr
    dl
  )
endif()