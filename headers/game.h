#pragma once
#include "client_management.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define UPPER 100
#define LOWER 0
#define ATTEMPTS 10

#define BUFF_LEN 1024

void handleGuess(Client *clients, int client_index, char buffer[], long *answer);

void updateClientStatus(Client *client, int win);

int isGuessCorrect(int guess, long answer);

void sendAttemptsLeftMessage(Client client, char* buffer);

void sendClientWinMessage(Client *clients, Client *client);

void sendGameOverMessage(Client client);

void resetGame(long *answer, Client *client);

long randomize(int lower, int upper);

int isDigit(char *buffer);
