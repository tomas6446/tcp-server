#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>

#define BUFF_LEN 1024
#define MAX_CLIENTS 10

int findEmptyUser(const int client_sockets[]) {
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == -1) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    unsigned int port;
    unsigned int addrLen;

    int l_socket;
    int client_sockets[MAX_CLIENTS];
    int maxfd = 0;
    int i;

    fd_set read_set;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    char buffer[BUFF_LEN];

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
        return -1;
    }
    port = atoi(argv[1]);
    if ((port < 1) || (port > 65535)) {
        fprintf(stderr, "ERROR #1: invalid port specified.\n");
        return -1;
    }

    if ((l_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "ERROR #2: cannot create listening socket.\n");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(l_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "ERROR #3: bind listening socket.\n");
        return -1;
    }

    if (listen(l_socket, 5) < 0) {
        fprintf(stderr, "ERROR #4: error in listen().\n");
        return -1;
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = -1;
    }


    while(1) {
        FD_ZERO(&read_set);
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1) {
                FD_SET(client_sockets[i], &read_set);
                if (client_sockets[i] > maxfd) {
                    maxfd = client_sockets[i];
                }
            }
        }
        FD_SET(l_socket, &read_set);
        if (l_socket > maxfd) {
            maxfd = l_socket;
        }

        select(maxfd + 1, &read_set, NULL, NULL, NULL);

        if (FD_ISSET(l_socket, &read_set)) {
            int client_id = findEmptyUser(client_sockets);
            if (client_id != -1) {
                addrLen = sizeof(client_addr);
                memset(&client_addr, 0, addrLen);
                client_sockets[client_id] = accept(l_socket,
                                                   (struct sockaddr *) &client_addr, &addrLen);
                printf("Connected:  %s\n", inet_ntoa(client_addr.sin_addr));
            }
        }
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1) {
                if (FD_ISSET(client_sockets[i], &read_set)) {
                    memset(&buffer, 0, BUFF_LEN);
                    int r_len = recv(client_sockets[i], &buffer, BUFF_LEN, 0);

                    int j;
                    for (j = 0; j < MAX_CLIENTS; j++) {
                        if (client_sockets[j] != -1) {
                            int w_len = send(client_sockets[j], buffer, r_len, 0);
                            if (w_len <= 0) {
                                close(client_sockets[j]);
                                client_sockets[j] = -1;
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}
