cmake_minimum_required(VERSION 3.20)
project(http_proxy_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(proxy
    src/main.cpp
    src/socket_utils.cpp
    src/http_parser.cpp
)

target_include_directories(proxy
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(proxy
    PRIVATE
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        fmt::fmt
)

if (WIN32)
    target_link_libraries(proxy PRIVATE ws2_32)
endif()
