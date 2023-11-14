#include "../headers/game.h"

long randomize(int lower, int upper) {
    srand(time(NULL));
    return (rand() % (upper - lower + 1)) + lower;
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

int isGuessCorrect(int guess, long answer) {
    if (guess < answer) {
        return -1; // Guess is lower than the answer
    } else if (guess > answer) {
        return 1; // Guess is higher than the answer
    } else {
        return 0; // Guess is correct
    }
}

void updateClientStatus(Client *client, int win) {
    if (win) {
        client->winCount++;
    }
    client->attempts--;
}


void sendAttemptsLeftMessage(Client client, char *buffer) {
    sprintf(buffer, "%s (%d attempts left)\n", buffer, client.attempts);
    sendMessage(client, buffer);
}

void resetGame(long *answer, Client *client) {
    *answer = randomize(LOWER, UPPER);
    printf("Answer: %ld\n", *answer);
    char buffer[] = "You have: 10 attempts\n";
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client[i].socket_fd != -1) {
            client->attempts = ATTEMPTS;
            sendMessage(client[i], buffer);
        }
    }
}

void sendClientWinMessage(Client *clients, Client *winner) {
    updateClientStatus(winner, 1);
    char *buffer = malloc(sizeof(char) * BUFF_LEN);
    sprintf(buffer, "%s won the game. Win count = [%d].\n",
            winner->username,
            winner->winCount);
    messageAllClients(clients, buffer);
}

void sendGameOverMessage(Client client) {
    char buffer[] = "No attempts left\n";
    sendMessage(client, buffer);
}

// Refactored handleGuess function
void handleGuess(Client *clients, int client_index, char buffer[], long *answer) {
    Client *client = &clients[client_index];

    // Game over condition
    if (client->attempts == 1) {
        sendGameOverMessage(*client);
        return;
    }

    if (!isDigit(buffer)) {
        strcpy(buffer, "Not a number\n");
        sendMessage(*client, buffer);
        return;
    }

    int guess = atoi(buffer);
    int guessResult = isGuessCorrect(guess, *answer);

    if (guessResult == 0) {
        sendClientWinMessage(clients, client);
        resetGame(answer, client);
    } else {
        strcpy(buffer, guessResult < 0 ? "HIGHER" : "LOWER");
        updateClientStatus(client, 0);
        sendAttemptsLeftMessage(*client, buffer);
        printf("Message received from %s: %d %s", client->username, guess, buffer);
    }
}
