cmake_minimum_required(VERSION 3.10)
project(zmqhb)

set(CMAKE_CXX_STANDARD 17)

add_executable(server.out server.cpp)
add_executable(client.out client.cpp)

target_link_libraries(server.out PUBLIC zmq pthread ssl crypto)
target_link_libraries(client.out PUBLIC zmq ssl crypto)