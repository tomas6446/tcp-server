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

void updateMaxFd(int fd, int *maxfd);

void processSelect(Client *clients, long *answer, char *buffer, Connection *serverConnection);

void addClientSocketReadSet(const Client *clients, int *maxfd, fd_set *read_set);

void acceptConnection(int client_id, int serverSocket, Client *clients);

void initializeClient(int client_id, Client *clients, char *buffer);

void serverRun(char *argv[]);

void acceptServerConnection(Client *clients, char *buffer, Connection *serverConnection);

void handleClientInput(Client *clients, long *answer, char *buffer, const Connection *serverConnection);

void handle_signal(int sig);

int main(int argc, char *argv[]) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_signal;
    sigaction(SIGINT, &sa, NULL);

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
        exit(1);
    }

    /*
     * Main loop to accept incoming connections and read messages from clients
     */
    serverRun(argv);
    return 0;
}

void updateMaxFd(int fd, int *maxfd) {
    if (fd > *maxfd) {
        *maxfd = fd;
    }
}

void addClientSocketReadSet(const Client *clients, int *maxfd, fd_set *read_set) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_fd != -1) {
            FD_SET(clients[i].socket_fd, read_set);
            updateMaxFd(clients[i].socket_fd, maxfd);
        }
    }
}

void processSelect(Client *clients, long *answer, char *buffer, Connection *serverConnection) {
    if (FD_ISSET(serverConnection->socket, &serverConnection->read_set)) {
        acceptServerConnection(clients, buffer, serverConnection);
    }
    handleClientInput(clients, answer, buffer, serverConnection);
}

void acceptConnection(int client_id, int serverSocket, Client *clients) {
    unsigned int addr_length = sizeof(clients[client_id].addr);
    memset(&clients[client_id].addr, 0, addr_length);
    clients[client_id].socket_fd = accept(serverSocket, (struct sockaddr *) &clients[client_id].addr, &addr_length);
    printf("Connected: %s with ", inet_ntoa(clients[client_id].addr.sin_addr));
}

void initializeClient(int client_id, Client *clients, char *buffer) {
    recv(clients[client_id].socket_fd, buffer, USERNAME_LEN, 0);
    if (strncmp(buffer, "username:", 9) == 0) {
        clients[client_id].username = malloc(sizeof(char) * USERNAME_LEN);
        strcpy(clients[client_id].username, buffer + 9);
        printf("username: %s\n", clients[client_id].username);

        buffer = "Welcome to the game! You have: 10 attempts\n";
        sendMessage(clients[client_id], buffer);
    }
}


void acceptServerConnection(Client *clients, char *buffer, Connection *serverConnection) {
    int client_id = findEmptyUser(clients);
    if (client_id != -1) {
        acceptConnection(client_id, serverConnection->socket, clients);
        initializeClient(client_id, clients, buffer);
    }
}

void handleClientInput(Client *clients, long *answer, char *buffer, const Connection *serverConnection) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_fd != -1 && FD_ISSET(clients[i].socket_fd, &(*serverConnection).read_set)) {
            memset(buffer, 0, BUFF_LEN);
            recv(clients[i].socket_fd, buffer, BUFF_LEN, 0);

            if (strncmp(buffer, "/q", 2) == 0) {
                disconnectClient(clients, i);
            } else if (strncmp(buffer, "/m", 2) == 0) {
                messageAllClients(clients, buffer);
            } else {
                handleGuess(clients, i, buffer, answer);
            }
        }
    }
}

void serverRun(char *argv[]) {
    printf("Server running...\n");

    Client *clients = initClients();
    Connection *serverConnection = createServerConnection(argv);
    int maxfd = serverConnection->socket;
    char buffer[BUFF_LEN];
    long *answer = initGame();

    while (server_running) {
        FD_ZERO(&serverConnection->read_set);

        addClientSocketReadSet(clients, &maxfd, &serverConnection->read_set);
        FD_SET(serverConnection->socket, &serverConnection->read_set);

        select(maxfd + 1, &serverConnection->read_set, NULL, NULL, NULL);
        processSelect(clients, answer, buffer, serverConnection);
    }

    cleanupClients(clients);
    close(serverConnection->socket);
    free(answer);
}

void handle_signal(int sig) {
    if (sig == SIGINT) {
        server_running = 0;
    }
}
