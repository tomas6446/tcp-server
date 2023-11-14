#include "../headers/client_management.h"
#include "../headers/game.h"


void cleanupClients(Client *clients) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_fd != -1) {
            close(clients[i].socket_fd); // Close the socket
            clients[i].socket_fd = -1;
        }
        free(clients[i].username); // Free the username memory
        clients[i].username = NULL;
    }
}

void disconnectClient(Client *clients, int client_index) {
    printf("Disconnected: %s(%d) \n", clients[client_index].username, clients[client_index].socket_fd);
    close(clients[client_index].socket_fd);
    free(clients[client_index].username);
    clients[client_index].username = NULL;
    clients[client_index].socket_fd = -1;
}

Client* initClients() {
    Client *clients = malloc(MAX_CLIENTS * sizeof(Client));
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].username = malloc(sizeof(char) * 11);
        clients[i].attempts = ATTEMPTS;
        clients[i].won = 0;
        clients[i].win_count = 0;
        clients[i].socket_fd = -1;
    }
    return clients;
}

int findEmptyUser(Client *client) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client[i].socket_fd == -1) {
            return i;
        }
    }
    return -1;
}


void messageAllClients(Client *clients, const char *buffer) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sendMessage(clients[i], buffer + 2);
    }
}

void sendMessage(Client client_socket, const char *buffer) {
    long send_length = send(client_socket.socket_fd, buffer, strlen(buffer), 0);
    if (send_length <= 0) {
        close(client_socket.socket_fd);
        client_socket.socket_fd = -1;
    }
}
