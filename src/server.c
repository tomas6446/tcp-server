#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "../headers/connection.h"
#include "../headers/client_management.h"
#include "../headers/game.h"

volatile sig_atomic_t server_running = 1;

void acceptServerConnection(struct sockaddr_in *client_addr, Client *clients, char *buffer, const Connection *serverConnection);

void handleClientInput(Client *clients, long *answer, char *buffer, const Connection *serverConnection);

void addClientSocketReadSet(const Client *clients, int *maxfd, Connection *serverConnection);

void serverRun(Client *clients, Connection *serverConnection, long *answer);

void handle_signal(int sig) {
    if (sig == SIGINT) {
        server_running = 0;
    }
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_signal;
    sigaction(SIGINT, &sa, NULL);

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
        exit(1);
    }

    Client *clients = initClients();
    Connection *serverConnection = createServerConnection(argv);
    long *answer = initGame();

    /*
     * Main loop to accept incoming connections and read messages from clients
     */
    serverRun(clients, serverConnection, answer);
    return 0;
}

void serverRun(Client *clients, Connection *serverConnection, long *answer) {
    int maxfd = 0;
    struct sockaddr_in client_addr;
    char buffer[BUFF_LEN];
    while (server_running) {
        // clear the read_set
        FD_ZERO(&(*serverConnection).read_set);

        // Add each connected client socket to the read_set and update maxfd
        addClientSocketReadSet(clients, &maxfd, serverConnection);

        // Add the listening socket to the read_set and update maxfd
        FD_SET((*serverConnection).socket, &(*serverConnection).read_set);
        if ((*serverConnection).socket > maxfd) {
            maxfd = (*serverConnection).socket;
        }

        // Wait for activity on any of the file descriptors in the read_set
        select(maxfd + 1, &(*serverConnection).read_set, NULL, NULL, NULL);

        // Check if a new client is connecting and accept the serverConnection
        acceptServerConnection(&client_addr, clients, buffer, serverConnection);
        handleClientInput(clients, answer, buffer, serverConnection);
    }
    cleanupClients(clients);
    close((*serverConnection).socket);
    free(answer);
}

void addClientSocketReadSet(const Client *clients, int *maxfd, Connection *serverConnection) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_fd != -1) {
            FD_SET(clients[i].socket_fd, &(*serverConnection).read_set);
            if (clients[i].socket_fd > (*maxfd)) {
                (*maxfd) = clients[i].socket_fd;
            }
        }
    }
}

void handleClientInput(Client *clients, long *answer, char *buffer, const Connection *serverConnection) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_fd != -1 && FD_ISSET(clients[i].socket_fd, &(*serverConnection).read_set)) {
            memset(buffer, 0, BUFF_LEN);
            recv(clients[i].socket_fd, buffer, BUFF_LEN, 0);

            if (strncmp(buffer, "\\q\n", 2) == 0) {
                disconnectClient(clients, i);
            } else if (strncmp(buffer, "\\m\n", 2) == 0) {
                messageAllClients(clients, buffer);
            } else {
                handleGuess(clients, i, buffer, answer);
            }
        }
    }
}

void acceptServerConnection(struct sockaddr_in *client_addr, Client *clients, char *buffer, const Connection *serverConnection) {
    if (FD_ISSET((*serverConnection).socket, &(*serverConnection).read_set)) {
        int client_id = findEmptyUser(clients);
        if (client_id != -1) {
            unsigned int addr_length = sizeof(*client_addr);
            memset(client_addr, 0, addr_length);
            clients[client_id].socket_fd = accept((*serverConnection).socket,
                                                  (struct sockaddr *) client_addr, &addr_length);
            printf("Connected: %s with ", inet_ntoa((*client_addr).sin_addr));

            // receive username at the start of the client program
            recv(clients[client_id].socket_fd, buffer, USERNAME_LEN, 0);
            if (strncmp(buffer, "username:", 9) == 0) {
                strcpy(clients[client_id].username, buffer + 9);
                printf("username: %s\n", clients[client_id].username);
            }
        }
    }
}
