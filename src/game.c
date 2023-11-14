#include <sys/time.h>
#include "../headers/game.h"

long* initGame() {
    struct timeval time;
    gettimeofday(&time,NULL);
    srand((time.tv_sec * 1000) + (time.tv_usec / 1000));

    long *answer = malloc(sizeof(int));
    *answer = randomize(LOWER, UPPER);
    printf("The answer is %ld\n", *answer);
    return answer;
}

int isDigit(char *buffer) {
    buffer[strlen(buffer) - 1] = '\0';
    for (int i = 0; i < strlen(buffer); i++) {
        if (!isdigit(buffer[i])) {
            return 0;
        }
    }
    return 1;
}

long randomize(int lower, int upper) {
    return (rand() % (upper - lower + 1)) + lower;
}

void handleGuess(Client *clients, int client_index, char buffer[BUFF_LEN], long *answer) {
    int win_exists = 0;

    if (clients[client_index].attempts > 1 && !clients[client_index].won) {
        if (isDigit(buffer)) {
            int g = atoi(buffer);
            if (g < *answer) {
                strcpy(buffer, "HIGHER");
            } else if (g > *answer) {
                strcpy(buffer, "LOWER");
            } else {
                strcpy(buffer, "WIN");
                clients[client_index].won = 1;
                win_exists = 1;
            }
            clients[client_index].attempts--;
        } else {
            strcpy(buffer, "Not a number");
        }

        char attempts_left[20];
        sprintf(attempts_left, " (%d attempts left)\n", clients[client_index].attempts);
        strcat(buffer, attempts_left);
    } else if (clients[client_index].attempts == 1) {
        strcpy(buffer, "No attempts left\n");
    }
    sendMessage(clients[client_index], buffer);

    for (int j = 0; j < MAX_CLIENTS; j++) {
        if (clients[j].socket_fd != -1) {
            printf("Message received from %s: %s", clients[j].username, buffer);
            if (win_exists) {
                resetGame(answer, &clients[j], buffer);
            }
        }
    }
}

void resetGame(long *answer, Client *client, char buffer[BUFF_LEN]) {
    if (client->won) {
        client->win_count++;
        client->won = 0;
    }
    client->attempts = ATTEMPTS; // reset attempts
    *answer = randomize(LOWER, UPPER); // reset number

    sprintf(buffer, "%s won the game with win count of %d. New number generated\n",
            client->username,
            client->win_count);
    printf("The answer is %ld\n", *answer);
    sendMessage(*client, buffer);
}
