#include "../headers/connection.h"

unsigned int validatePort(unsigned int port) {
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

void createClientAddressFamily(Connection *connection) {
    memset(&(*connection).addr, 0, sizeof((*connection).addr));
    (*connection).addr.sin_family = AF_INET;
    (*connection).addr.sin_port = htons(connection->port);
}

void createServerAddressFamily(Connection *connection) {
    createClientAddressFamily(connection);
    (*connection).addr.sin_addr.s_addr = htonl(INADDR_ANY);
}


Connection createClientConnection(char *const *argv) {
    unsigned int port = atoi(argv[2]);
    Connection connection;
    connection.port = validatePort(port);
    connection.socket = createSocket();
    createClientAddressFamily(&connection);

    /*
     * Convert IP address from string to binary form
     * and store in addr.sin_addr
     */
    if (inet_aton(argv[1], &connection.addr.sin_addr) <= 0) {
        fprintf(stderr, "ERROR #3: Invalid remote IP address.\n");
        exit(1);
    }

    /*
     * Connect to server
     */
    if (connect(connection.socket, (struct sockaddr *) &connection.addr, sizeof(connection.addr)) < 0) {
        fprintf(stderr, "ERROR #4: error in connect().\n");
        exit(1);
    }

    return connection;
}

Connection createServerConnection(char *const *argv) {
    unsigned int port = atoi(argv[1]);
    Connection connection;
    connection.port = validatePort(port);
    connection.socket = createSocket();
    createServerAddressFamily(&connection);

    int yes = 1;
    if (setsockopt(connection.socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        fprintf(stderr, "ERROR #3: bind listening socket.\n");
        exit(1);
    }

    if (bind(connection.socket, (struct sockaddr *) &connection.addr, sizeof(connection.addr)) < 0) {
        fprintf(stderr, "ERROR #3: bind listening socket.\n");
        exit(1);
    }

    /*
     * Start listening for incoming connections
     */
    if (listen(connection.socket, 5) < 0) {
        fprintf(stderr, "ERROR #4: error in listen().\n");
        exit(1);
    }

    return connection;
}
