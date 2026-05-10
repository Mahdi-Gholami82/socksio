cmake_minimum_required(VERSION 3.31.6)

project(socksio VERSION 0.1 LANGUAGES CXX)

# export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  asio
  GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
  GIT_TAG        "asio-1-38-0"
)
FetchContent_MakeAvailable(asio)

set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include)

add_library(asio INTERFACE)
target_compile_features(asio INTERFACE cxx_std_20)
target_include_directories(asio SYSTEM INTERFACE ${ASIO_INCLUDE_DIR})


file(GLOB SRC_FILES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE asio)
target_include_directories(${PROJECT_NAME} PRIVATE include)