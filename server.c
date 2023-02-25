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


int main() {

    char *ip = "127.0.0.1";
    int port = 5566;

    int server_socket;
    int client_socket;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("[-] Socket error");
        exit(1);
    }
    printf("[+] TCP server socket created.\n");


    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (n < 0) {
        perror("[-] Bind error");
        exit(1);
    }
    printf("[+] Bind to the port number: %d\n", port);

    listen(server_socket, 5);
    printf("Listening...\n");

    while (1) {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &addr_size);
        printf("[+] Client connected.\n");
    }

    return 0;
}
