
#include <fcntl.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <slcurses.h>

#include "../headers/connection.h"

#define BUFF_LEN 1024
#define USERNAME_LEN 20

bool validateUsername(char username[20]);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "USAGE: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    Connection clientConnection = createClientConnection(argv);

    char username[USERNAME_LEN];
    char recv_buff[BUFF_LEN];
    char send_buff[BUFF_LEN];

    printf("Guess the Number\".\n"
           "The server chooses a random number between 1 and 100.\n"
           "The client connects to the server and is prompted to handle_guess the number.\n"
           "The client has 10 attempts to handle_guess the number.\n"
           "After each handle_guess, the server responds with either \"Higher\", \"Lower\", or \"Correct!\".\n"
           "If the client guesses the number correctly, they win the game."
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
    write(clientConnection.socket, username_message, sizeof(username_message));
    printf("Connected to server\n");

    memset(&send_buff, 0, BUFF_LEN);                                // clear memory for send buffer
    fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);  // make standard input non-blocking
    while (1) {
        // Create and initialize the read_set with socket and standard input
        FD_ZERO(&clientConnection.read_set);
        FD_SET(clientConnection.socket, &clientConnection.read_set);
        FD_SET(0, &clientConnection.read_set);   // 0 is the file descriptor for standard input

        // Monitor read_set for any incoming data
        select(clientConnection.socket + 1, &clientConnection.read_set, NULL, NULL, NULL);
        // Check if the incoming data is from the socket
        if (FD_ISSET(clientConnection.socket, &clientConnection.read_set)) {
            memset(&recv_buff, 0, BUFF_LEN);                   // clear the reception buffer
            read(clientConnection.socket, &recv_buff, BUFF_LEN);  // read the incoming message from socket

            printf("%s", recv_buff);
        } else if (FD_ISSET(0, &clientConnection.read_set)) {
            fgets(send_buff, BUFF_LEN, stdin);
            if (strncmp(send_buff, "/q", 2) == 0) {
                break;
            }
            write(clientConnection.socket, send_buff, sizeof(send_buff));
        }
    }

    close(clientConnection.socket);
    return 0;
}

bool validateUsername(char username[USERNAME_LEN]) {
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
