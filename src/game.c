#include "../headers/game.h"


long randomize(int lower, int upper) {
    srand(time(NULL));
    return (rand() % (upper - lower + 1)) + lower;
}

long *initGame() {
    long *answer = malloc(sizeof(long));
    *answer = randomize(LOWER, UPPER);
    printf("Answer: %ld\n", *answer);
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
        client->won++;
    }
    client->attempts--;
}


void sendAttemptsLeftMessage(Client client, char *buffer) {
    sprintf(buffer, "%s (%d attempts left)\n", buffer, client.attempts);
    sendMessage(client, buffer);
}

void resetGame(long *answer, Client *client, char buffer[]) {
    *answer = randomize(LOWER, UPPER);
    printf("Answer: %ld\n", *answer);
    client->attempts = ATTEMPTS;
    sendAttemptsLeftMessage(*client, buffer);
}

void sendWinMessage(Client *client, char *buffer) {
    strcpy(buffer, "WIN");
    updateClientStatus(client, 1);
    sendAttemptsLeftMessage(*client, buffer);
}

void sendGameOverMessage(Client client, char *buffer) {
    strcpy(buffer, "No attempts left\n");
    sendMessage(client, buffer);
}

void resetGameIfWinExists(long *answer, Client clients[], int win_exists, char buffer[]) {
    if (win_exists) {
        for (int j = 0; j < MAX_CLIENTS; j++) {
            if (clients[j].socket_fd != -1) {
                resetGame(answer, &clients[j], buffer);
            }
        }
    }
}

// Refactored handleGuess function
void handleGuess(Client *clients, int client_index, char buffer[], long *answer) {
    Client *client = &clients[client_index];
    int win_exists = 0;

    // Game over condition
    if (client->attempts == 1) {
        sendGameOverMessage(*client, buffer);
        return;
    }

    if (!isDigit(buffer)) {
        strcpy(buffer, "Not a number");
        sendMessage(*client, buffer);
        return;
    }

    int guess = atoi(buffer);
    int guessResult = isGuessCorrect(guess, *answer);

    if (guessResult == 0) {
        sendWinMessage(client, buffer);
        win_exists = 1;
    } else {
        strcpy(buffer, guessResult < 0 ? "HIGHER" : "LOWER");
        updateClientStatus(client, 0);
        sendAttemptsLeftMessage(*client, buffer);
    }

    // Broadcast game reset if there's a win
    resetGameIfWinExists(answer, clients, win_exists, buffer);
}
