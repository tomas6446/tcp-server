
#include <fcntl.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <slcurses.h>
#include <bits/signum-generic.h>
#include <signal.h>

#include "../headers/connection.h"

#define BUFF_LEN 1024
#define USERNAME_LEN 20

volatile sig_atomic_t client_running = 1;

bool validateUsername(const char *username);

void handleData(const Connection *connection, char *recv_buff, char *send_buff);

void handle_signal(int sig);

void runClient(char *argv[]);

int main(int argc, char *argv[]) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_signal;
    sigaction(SIGINT, &sa, NULL);


    runClient(argv);
    return 0;
}

void runClient(char *argv[]) {
    Connection *connection = createClientConnection(argv);

    char username[USERNAME_LEN];
    char recv_buff[BUFF_LEN];
    char send_buff[BUFF_LEN];

    printf("Guess the Number.\n"
           "The server chooses a random number between 1 and 100.\n"
           "The client connects to the server and is prompted to guess the number.\n"
           "The client has 10 attempts to guess the number.\n"
           "After each guess, the server responds with either \"Higher\", \"Lower\", or \"Correct!\".\n"
           "If the client guesses the number correctly, they win the game.\n"
           "OPTIONS:\n"
           "/q - to quit\n"
           "/m - to message all clients\n\n");

    do {
        printf("Username (3-11 char length): ");
        fgets(username, USERNAME_LEN, stdin);
        username[strlen(username) - 1] = '\0';
    } while (!validateUsername(username));


    char username_message[USERNAME_LEN];
    sprintf(username_message, "username:%s", username);
    write(connection->socket, username_message, sizeof(username_message));
    printf("%s. Connected to server\n", username);

    memset(&send_buff, 0, BUFF_LEN);                                // clear memory for send buffer
    fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);  // make standard input non-blocking
    while (client_running) {
        // Create and initialize the read_set with server_socket and standard input
        FD_ZERO(&connection->read_set);
        FD_SET(connection->socket, &connection->read_set);
        FD_SET(0, &connection->read_set);   // 0 is the file descriptor for standard input

        // Monitor read_set for any incoming data
        select(connection->socket + 1, &connection->read_set, NULL, NULL, NULL);
        // Check if the incoming data is from the server_socket
        handleData(connection, recv_buff, send_buff);
    }

    close(connection->socket);
}

void handleData(const Connection *connection, char *recv_buff, char *send_buff) {
    if (FD_ISSET(connection->socket, &connection->read_set)) {
        memset(recv_buff, 0, BUFF_LEN);                   // clear the reception buffer
        read(connection->socket, recv_buff, BUFF_LEN);  // read the incoming message from server_socket

        printf("%s", recv_buff);
    } else if (FD_ISSET(0, &connection->read_set)) {
        fgets(send_buff, BUFF_LEN, stdin);
        write(connection->socket, send_buff, sizeof(send_buff));

        if (strncmp(send_buff, "/q", 2) == 0) {
            shutdown(connection->socket, SHUT_RDWR);
            exit(0);
        }
    }
}

bool validateUsername(const char *username) {
    regex_t regex;
    if (regcomp(&regex, "^[A-Za-z0-9_]{3,11}$", REG_EXTENDED) == 0) {
        if (regexec(&regex, username, 0, NULL, 0) != 0) {
            printf("Invalid username\n");
            return 0;
        }

        regfree(&regex);
        return 1;
    } else {
        fprintf(stderr, "Could not compile regex\n");
        exit(EXIT_FAILURE);
    }
}

void handle_signal(int sig) {
    if (sig == SIGINT) {
        client_running = 0;
    }
}
