//
// Created by tomas
//

#ifdef _WIN32
#include <winsock2.h>
#else

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFLEN 1024

int main(int argc, char *argv[]) {

    char *ip = "127.0.0.1";
    int port;

    int server_socket;
    int client_socket;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    char buffer[BUFFLEN];
    int n;

    if (argc != 2) {
        printf("USAGE: %s <port>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);

    if ((port < 1) || (port > 65535)) {
        perror("[-] Invalid port specified.\n");
        exit(1);
    }

#ifdef _WIN32
    WSAStartup(MAKEWORD(2,2),&data);
#endif
    /*
     * Creating socket
     */
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[-] Socket error");
        exit(1);
    }
    printf("[+] TCP server socket created.\n");

    /*
     * Clean and fill up servers structure
     */
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);


    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("[-] Bind error");
        exit(1);
    }
    printf("[+] Bind to the port number: %d\n", port);


    if(listen(server_socket, 5) < 0) {
        perror("[-] Error listening.\n");
        exit(1);
    }

    printf("Listening...\n");
    while (1) {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &addr_size);
        printf("[+] Client connected.\n");

        bzero(buffer, 1024);
        recv(client_socket, buffer, sizeof(buffer), 0);
        printf("Client: %s\n", buffer);

        bzero(buffer, 1024);
        strcpy(buffer, "Hi, this is server.\n");
        printf("Server: %s\n", buffer);
        send(client_socket, buffer, strlen(buffer), 0);

        close(client_socket);

        printf("[+] Client disconnected.\n\n");
    }

    return 0;
}
