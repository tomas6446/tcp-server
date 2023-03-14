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
    int l_socket;                       // socket for connection waiting
    int client_sockets[MAX_CLIENTS];    // client sockets
    int maxfd = 0;

    fd_set read_set;
    struct sockaddr_in server_addr;     // server address structure
    struct sockaddr_in client_addr;     // client address structure
    char buffer[BUFF_LEN];

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[1]);
    if ((port < 1) || (port > 65535)) {
        fprintf(stderr, "ERROR #1: invalid port specified.\n");
        exit(1);
    }

    /*
     * Create the listening socket
     */
    if ((l_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "ERROR #2: cannot create listening socket.\n");
        exit(1);
    }

    /*
     * Bind the listening socket to the server address
     */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;                           // IP protocol
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // IP address
    server_addr.sin_port = htons(port);                // port
    if (bind(l_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "ERROR #3: bind listening socket.\n");
        exit(1);
    }

    /*
     * Start listening for incoming connections
     */
    if (listen(l_socket, 5) < 0) {
        fprintf(stderr, "ERROR #4: error in listen().\n");
        exit(1);
    }

    /*
     * Initialize client_sockets array
     */
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = -1;
    }

    /*
     * Main loop to accept incoming connections and read messages from clients
     */
    while (1) {
        // clear the read_set
        FD_ZERO(&read_set);

        // Add each connected client socket to the read_set and update maxfd
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1) {
                FD_SET(client_sockets[i], &read_set);
                if (client_sockets[i] > maxfd) {
                    maxfd = client_sockets[i];
                }
            }
        }

        // Add the listening socket to the read_set and update maxfd
        FD_SET(l_socket, &read_set);
        if (l_socket > maxfd) {
            maxfd = l_socket;
        }

        // Wait for activity on any of the file descriptors in the read_set
        select(maxfd + 1, &read_set, NULL, NULL, NULL);

        // Check if a new client is connecting and accept the connection
        if (FD_ISSET(l_socket, &read_set)) {
            int client_id = findEmptyUser(client_sockets);
            if (client_id != -1) {
                unsigned int addr_length = sizeof(client_addr);
                memset(&client_addr, 0, addr_length);
                client_sockets[client_id] = accept(l_socket,
                                                   (struct sockaddr *) &client_addr, &addr_length);
                printf("Connected:  %s\n", inet_ntoa(client_addr.sin_addr));
            }
        }
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1) {
                if (FD_ISSET(client_sockets[i], &read_set)) {
                    memset(&buffer, 0, BUFF_LEN);
                    long recv_length = recv(client_sockets[i], &buffer, BUFF_LEN, 0);

                    // Check if client has disconnected
                    if (recv_length <= 0) {
                        printf("Disconnected: %s\n", inet_ntoa(client_addr.sin_addr));
                        close(client_sockets[i]); // close client socket
                        client_sockets[i] = -1;
                    } else {
                        int j;
                        for (j = 0; j < MAX_CLIENTS; j++) {
                            if (client_sockets[j] != -1) {
                                printf("Message received from %s: %s\n", inet_ntoa(client_addr.sin_addr), buffer);
                                long send_length = send(client_sockets[j], buffer, recv_length, 0);
                                if (send_length <= 0) {
                                    close(client_sockets[j]);
                                    client_sockets[j] = -1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}
