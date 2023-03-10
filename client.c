
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFF_LEN 1024

int main(int argc, char *argv[]) {
    unsigned int port;
    int server_socket;
    struct sockaddr_in server_addr;     // struct to hold server address information
    fd_set read_set;

    char recv_buff[BUFF_LEN];
    char send_buff[BUFF_LEN];

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
    printf("OPTIONS:\n"
           "/q - to quit\n");
    printf("Username: ");
    fgets(username, BUFF_LEN, stdin);
    int len = strlen(username);
    if (len > 0 && username[len - 1] == '\n') {
        username[len - 1] = '\0';
    }

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
            read(0, &send_buff, sizeof(send_buff));     // read the input message from standard input
            if (strncmp(send_buff, "/q\n", 2) == 0) {
                printf("Quitting...");

                char disconnect_message[BUFF_LEN];
                sprintf(disconnect_message, "%s %s\n", username, "has been disconnected.");
                write(server_socket, disconnect_message, sizeof(disconnect_message));

                // close the server socket and exit the program
                close(server_socket);
                exit(0);
            } else {
                char message[BUFF_LEN];
                sprintf(message, "%s: %s", username, send_buff);    // format message with username
                write(server_socket, message, sizeof(message));
            }
        }
    }

    close(server_socket);
    return 0;
}