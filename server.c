#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <slcurses.h>
#include <ctype.h>

#define BUFF_LEN 1024
#define USERNAME_LEN 20
#define MAX_CLIENTS 10

// GAME PARAM
#define UPPER 100
#define LOWER 0
#define ATTEMPTS 10

typedef struct {
    int socket_fd;
    int attempts;
    int win_count;
    int won;
    char *username;
} Client;

int randomize(int lower, int upper);

void guess(Client *p_clients, int client_index, char buffer[BUFF_LEN], int *p_answer);

bool isDigit(char *buffer);

void send_message(Client client_socket, const char *buffer);

int findEmptyUser(Client client_sockets[]) {
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i].socket_fd == -1) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    unsigned int port;
    unsigned int addr_length;

    Client client_sockets[MAX_CLIENTS];
    Client *p_clients = client_sockets;


    for(int i = 0; i < MAX_CLIENTS; i++) {
        p_clients[i].username = malloc(sizeof(char) * 11);
        p_clients[i].attempts = ATTEMPTS;
        p_clients[i].won = 0;
    }


    int l_socket;
    int maxfd = 0;
    int i;

    fd_set read_set;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
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
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int yes = 1;
    if (setsockopt(l_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        fprintf(stderr, "ERROR #3: bind listening socket.\n");
        exit(1);
    }

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
    for (i = 0; i < MAX_CLIENTS; i++) {
        p_clients[i].socket_fd = -1;
    }

    /*
     * Main loop to accept incoming connections and read messages from clients
     */
    srand(time(0));
    int answer = randomize(LOWER, UPPER);
    int *p_answer = &answer;
    printf("The answer is %d", answer);

    while (1) {
        // clear the read_set
        FD_ZERO(&read_set);

        // Add each connected client socket to the read_set and update maxfd
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (p_clients[i].socket_fd != -1) {
                FD_SET(p_clients[i].socket_fd, &read_set);
                if (p_clients[i].socket_fd > maxfd) {
                    maxfd = p_clients[i].socket_fd;
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
            int client_id = findEmptyUser(p_clients);
            if (client_id != -1) {
                addr_length = sizeof(client_addr);
                memset(&client_addr, 0, addr_length);
                p_clients[client_id].socket_fd = accept(l_socket,
                                                             (struct sockaddr *) &client_addr, &addr_length);
                printf("Connected:  %s\n", inet_ntoa(client_addr.sin_addr));

                // receive username at the start of the client program
                recv(p_clients[client_id].socket_fd, &buffer, USERNAME_LEN, 0);
                if (strncmp(buffer, "username:", 9) == 0) {
                    char username[11];
                    strcpy(username, buffer + 9);
                    p_clients[client_id].username = username;
                }
            }
        }
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (p_clients[i].socket_fd != -1) {
                if (FD_ISSET(p_clients[i].socket_fd, &read_set)) {
                    memset(&buffer, 0, BUFF_LEN);
                    long recv_length = recv(p_clients[i].socket_fd, &buffer, BUFF_LEN, 0);

                    // Check if client has disconnected
                    if (recv_length <= 0 || strncmp(buffer, "\\q\n", 2) == 0) {
                        printf("Disconnected: %s(%s) \n", p_clients[i].username, inet_ntoa(client_addr.sin_addr));
                        close(p_clients[i].socket_fd); // close client socket
                        p_clients[i].socket_fd = -1;
                    } else {
                        guess(p_clients, i, buffer, p_answer);
                    }
                }
            }
        }
    }

    return 0;
}

void guess(Client *p_clients, int client_index, char buffer[BUFF_LEN], int *p_answer) {
    char winner[USERNAME_LEN];  // winner username

    if (p_clients[client_index].attempts > 1 && !p_clients[client_index].won) {
        if (isDigit(buffer)) {
            int g = atoi(buffer);
            if (g < *p_answer) {
                strcpy(buffer, "HIGHER");
            } else if (g > *p_answer) {
                strcpy(buffer, "LOWER");
            } else {
                strcpy(buffer, "WIN");
                strcpy(winner, p_clients[client_index].username);
                p_clients[client_index].won = 1;
            }
            p_clients[client_index].attempts--;
        } else {
            strcpy(buffer, "Not a number");
        }

        char attempts_left[20];
        sprintf(attempts_left, " (%d attempts left)\n", p_clients[client_index].attempts);
        strcat(buffer, attempts_left);
    } else if (p_clients[client_index].attempts == 1) {
        strcpy(buffer, "No attempts left\n");
    }
    send_message(p_clients[client_index], buffer);

//    for (int j = 0; j < MAX_CLIENTS; j++) {
//        if (p_clients[j].socket_fd != -1) {
//            printf("Message received from %s: %s", p_clients[j].username, buffer);
//            if (p_clients[j].won) {
//                sprintf(buffer, "%s won the game with win count of %d. New number generated\n",
//                        winner,
//                        p_clients[j].win_count);
//                *p_answer = randomize(LOWER, UPPER); // reset number
//                p_clients[j].attempts = ATTEMPTS; // reset attempts
//                p_clients[j].win_count++;
//
//                send_message(p_clients[j], buffer);
//            }
//        }
//    }
}

void send_message(Client client_socket, const char *buffer) {
    long send_length = send(client_socket.socket_fd, buffer, strlen(buffer), 0);
    if (send_length <= 0) {
        close(client_socket.socket_fd);
        client_socket.socket_fd = -1;
    }
}

bool isDigit(char *buffer) {
    buffer[strlen(buffer) - 1] = '\0';
    for (int i = 0; i < strlen(buffer); i++) {
        if (!isdigit(buffer[i])) {
            return 0;
        }
    }
    return 1;
}

int randomize(int lower, int upper) {
    return (rand() % (upper - lower + 1)) + lower;
}
