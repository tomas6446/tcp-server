cmake_minimum_required(VERSION 3.0)
project(tcp_server C)

set(CMAKE_C_STANDARD 11)

add_executable(tcp_server server.c client.c)
