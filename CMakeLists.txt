cmake_minimum_required(VERSION 3.0)
project(tcp_server C)

set(CMAKE_C_STANDARD 11)

add_executable(client
        src/client.c
        src/connection.c
        headers/connection.h
)

add_executable(server
        src/server.c
        src/connection.c
        headers/connection.h
        src/game.c
        headers/game.h
        src/client_management.c
        headers/client_management.h
        src/client.c
)
