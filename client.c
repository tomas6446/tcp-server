
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
    struct sockaddr_in server_addr;
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

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "ERROR #2: cannot create socket.\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);


    if (inet_aton(argv[1], &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "ERROR #3: Invalid remote IP address.\n");
        exit(1);
    }
    if (connect(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "ERROR #4: error in connect().\n");
        exit(1);
    }

    char username[BUFF_LEN];
    printf("OPTIONS:\n"
           "/q - to quit\n");
    printf("Username: %s", username);
    scanf("%s", username);

    memset(&send_buff, 0, BUFF_LEN);
    fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);
    while (1) {

        FD_ZERO(&read_set);
        FD_SET(server_socket, &read_set);
        FD_SET(0, &read_set);

        select(server_socket + 1, &read_set, NULL, NULL, NULL);
        if (FD_ISSET(server_socket, &read_set)) {
            memset(&recv_buff, 0, BUFF_LEN);
            read(server_socket, &recv_buff, BUFF_LEN);
            printf("%s", recv_buff);
        } else if (FD_ISSET(0, &read_set)) {
            read(0, &send_buff, sizeof(send_buff));
            if (strcmp(send_buff, "/q") == 0) {
                char disconnect[100];
                strcpy(disconnect, username);
                strcat(disconnect, " has been disconnected.");

                write(server_socket, disconnect, sizeof(disconnect));
                close(server_socket);
                exit(1);
            } else {
                char message[100];
                strcpy(message, username);
                strcat(message, ": ");
                strcat(message, send_buff);

                write(server_socket, message, sizeof(message));
            }
        }
    }

    close(server_socket);
    return 0;
}