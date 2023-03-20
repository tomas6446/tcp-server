
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFF_LEN 1024
#define USERNAME_LEN 20

int main(int argc, char *argv[]) {
    unsigned int port;
    int server_socket;

    struct sockaddr_in server_addr;     // struct to hold server address information
    fd_set read_set;

    if (argc != 3) {
        fprintf(stderr, "USAGE: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[2]);
    if ((port < 1) || (port > 65535)) {
        printf("ERROR #1: invalid port specified.\n");
        exit(1);
    }

    /*
     * Create server socket
     */
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "ERROR #2: cannot create socket.\n");
        exit(1);
    }

    /*
     * Clear memory for server struct
     * and specify address family and port number
     */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    /*
     * Convert IP address from string to binary form
     * and store in server_addr.sin_addr
     */
    if (inet_aton(argv[1], &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "ERROR #3: Invalid remote IP address.\n");
        exit(1);
    }

    /*
     * Connect to server
     */
    if (connect(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "ERROR #4: error in connect().\n");
        exit(1);
    }

    char username[BUFF_LEN];
    char recv_buff[BUFF_LEN];
    char send_buff[BUFF_LEN];
    printf("Guess the Number\".\n"
           "The server chooses a random number between 1 and 100.\n"
           "The client connects to the server and is prompted to guess the number.\n"
           "The client has 10 attempts to guess the number.\n"
           "After each guess, the server responds with either \"Higher\", \"Lower\", or \"Correct!\".\n"
           "If the client guesses the number correctly, they win the game."
           "OPTIONS:\n"
           "/q - to quit\n"
           "/m - to message all clients\n\n");
    printf("Username: ");
    fgets(username, BUFF_LEN, stdin);
    int len = strlen(username);
    if (len > 0 && username[len - 1] == '\n') {
        username[len - 1] = '\0';
    }

    char username_message[USERNAME_LEN];
    sprintf(username_message, "username:%s", username);
    write(server_socket, username_message, sizeof(username_message));

    memset(&send_buff, 0, BUFF_LEN);                                // clear memory for send buffer
    fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);  // make standard input non-blocking
    while (1) {
        // Create and initialize the read_set with server_socket and standard input
        FD_ZERO(&read_set);
        FD_SET(server_socket, &read_set);
        FD_SET(0, &read_set);   // 0 is the file descriptor for standard input

        // Monitor read_set for any incoming data
        select(server_socket + 1, &read_set, NULL, NULL, NULL);

        // Check if the incoming data is from the server_socket
        if (FD_ISSET(server_socket, &read_set)) {
            memset(&recv_buff, 0, BUFF_LEN);                   // clear the reception buffer
            read(server_socket, &recv_buff, BUFF_LEN);  // read the incoming message from server_socket

            printf("%s", recv_buff);
        } else if (FD_ISSET(0, &read_set)) {
            fgets(send_buff, BUFF_LEN, stdin);

            write(server_socket, send_buff, sizeof(send_buff));
        }
    }

    close(server_socket);
    return 0;
}