cmake_minimum_required(VERSION 3.28)
project(CppChat)

set(CMAKE_CXX_STANDARD 17)

add_executable(CppChat server.cpp)

target_link_libraries(CppChat wsock32 ws2_32)