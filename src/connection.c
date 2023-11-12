#include "../headers/connection.h"

unsigned int validatePort(int argc, char *const *argv) {
    if (argc != 3) {
        fprintf(stderr, "USAGE: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    unsigned int port = atoi(argv[2]);
    if ((port < 1) || (port > 65535)) {
        printf("ERROR #1: invalid port specified.\n");
        exit(1);
    }
    return port;
}

int createSocket() {
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "ERROR #2: cannot create socket.\n");
        exit(1);
    }
    return server_socket;
}

void createAddressFamily(unsigned int port, Connection *connection) {
    memset(&(*connection).server_addr, 0, sizeof((*connection).server_addr));
    (*connection).server_addr.sin_family = AF_INET;
    (*connection).server_addr.sin_port = htons(port);
}

Connection createClientConnection(int argc, char *const *argv) {
    Connection connection;
    connection.port = validatePort(argc, argv);
    connection.server_socket = createSocket();
    createAddressFamily(connection.port, &connection);

    /*
     * Convert IP address from string to binary form
     * and store in server_addr.sin_addr
     */
    if (inet_aton(argv[1], &connection.server_addr.sin_addr) <= 0) {
        fprintf(stderr, "ERROR #3: Invalid remote IP address.\n");
        exit(1);
    }

    /*
     * Connect to server
     */
    if (connect(connection.server_socket, (struct sockaddr *) &connection.server_addr, sizeof(connection.server_addr)) < 0) {
        fprintf(stderr, "ERROR #4: error in connect().\n");
        exit(1);
    }

    return connection;
}
