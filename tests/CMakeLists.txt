cmake_minimum_required(VERSION 3.14)

project(test_nudock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(NuDock REQUIRED)

add_executable(test_server server.cpp)
target_link_libraries(test_server PRIVATE NuDock::nudock)

add_executable(test_client client.cpp)
target_link_libraries(test_client PRIVATE NuDock::nudock)
