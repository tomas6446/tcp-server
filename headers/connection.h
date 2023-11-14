#pragma once

#include <sys/select.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct {
    unsigned int port;
    int socket;

    struct sockaddr_in addr;     // struct to hold server address information
    fd_set read_set;
} Connection;

int createSocket();

unsigned int validatePort(unsigned int port);

void createClientAddressFamily(Connection *connection);

void createServerAddressFamily(Connection *connection);

Connection createClientConnection(char *const *argv);

Connection createServerConnection(char *const *argv);
