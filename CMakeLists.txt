cmake_minimum_required(VERSION 3.31.6)

project(socksd VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MINGW OR WIN32)
    add_compile_definitions(
        _WIN32_WINNT=0x0601
        WINVER=0x0601
        _WIN32_IE=0x0601
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

include(FetchContent)

FetchContent_Declare(
  asio
  GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
  GIT_TAG        "asio-1-38-0"
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(asio)

set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include)

add_library(asio INTERFACE)
target_compile_features(asio INTERFACE cxx_std_20)
target_include_directories(asio SYSTEM INTERFACE "${ASIO_INCLUDE_DIR}")


file(GLOB SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE asio)
target_include_directories(${PROJECT_NAME} PRIVATE include)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        ws2_32
        wsock32  
    )
endif()
if(MINGW)
    message("Using MINGW")
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++exp)
    target_link_options(${PROJECT_NAME} PRIVATE
        -static
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message("Using GCC")
    target_link_options(${PROJECT_NAME} PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()
