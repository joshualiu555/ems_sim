include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/d72f9c8aea6817cdf1ca0ac10887f328de7f3da2.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
  libpqxx
  URL https://github.com/jtv/libpqxx/archive/8.0.1.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
  json 
  URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
  asio 
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git 
  GIT_TAG asio-1-38-0
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googletest libpqxx json asio)

add_library(asio_standalone INTERFACE)

target_include_directories(asio_standalone INTERFACE ${asio_SOURCE_DIR}/asio/include)

target_compile_definitions(asio_standalone INTERFACE ASIO_STANDALONE)
