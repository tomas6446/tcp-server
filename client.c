//
// Created by tomas
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFFLEN 1024

int main(int argc, char *argv[]) {
    char *ip = "127.0.0.1";
    unsigned int port;

    int sock = 0;
    struct sockaddr_in addr;
    socklen_t addr_size;
    char buffer[BUFFLEN];

    if (argc != 3) {
        fprintf(stderr, "USAGE: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[2]);

    if ((port < 1) || (port > 65535)) {
        perror("[-] Invalid port specified.\n");
        exit(1);
    }

#ifdef _WIN32
    WSAStartup(MAKEWORD(2,2),&data);
#endif
    if (socket(AF_INET, SOCK_STREAM, 0) < 0) {
        perror("[-] Socket error");
        exit(1);
    }
    printf("[+] TCP server sock created.\n");


    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

#ifdef _WIN32
    servaddr.sin_addr.s_addr = inet_addr(ip);
#else
    if (inet_aton(ip, &addr.sin_addr) <= 0) {
        perror("[-] Invalid remote IP address.\n");
        exit(1);
    }
#endif

    if (connect(sock, (struct sockaddr *) &addr, sizeof addr) < 0) {
        perror("[-] Error connecting to the server.\n");
    }
    printf("Connected to the server.\n");

    bzero(buffer, 1024);
    strcpy(buffer, "Hello, this is client.\n");
    printf("Client: %s\n", buffer);
    send(sock, buffer, strlen(buffer), 0);

    bzero(buffer, 1024);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    close(sock);

    printf("Disconnected from the server.\n");
    return 0;
}