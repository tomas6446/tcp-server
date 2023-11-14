#pragma once

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

#define MAX_CLIENTS 10
#define USERNAME_LEN 20

typedef struct {
    int socket_fd;
    int attempts;
    int win_count;
    int won;
    char *username;
} Client;

Client* initClients();

int findEmptyUser(Client *clients);

void disconnectClient(Client *clients, int client_index);

void cleanupClients(Client *clients);

void messageAllClients(Client *clients, const char *buffer);

void sendMessage(Client client_socket, const char *buffer);
