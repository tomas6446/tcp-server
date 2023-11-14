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
)

add_custom_target(
        run_client
        COMMAND ./client 127.0.0.1 5000
        DEPENDS client
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

add_custom_target(
        run_server
        COMMAND ./server 5000
        DEPENDS server
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)
