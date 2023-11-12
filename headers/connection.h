#pragma once

#include <sys/select.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct {
    unsigned int port;
    int server_socket;

    struct sockaddr_in server_addr;     // struct to hold server address information
    fd_set read_set;
} Connection;

unsigned int validatePort(int argc, char *const *argv);

int createSocket();

void createAddressFamily(unsigned int port, Connection *connection);

Connection createClientConnection(int argc, char *const *argv);
