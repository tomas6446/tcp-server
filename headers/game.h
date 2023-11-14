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

long *initGame();

void handleGuess(Client *clients, int client_index, char buffer[], long *answer);

void updateClientStatus(Client *client, int win);

int isGuessCorrect(int guess, long answer);

void sendAttemptsLeftMessage(Client client, char *buffer);

void sendWinMessage(Client *client, char *buffer);

void sendGameOverMessage(Client client, char *buffer);

void resetGameIfWinExists(long *answer, Client clients[], int win_exists, char buffer[]);

void resetGame(long *answer, Client *client, char buffer[]);

long randomize(int lower, int upper);

int isDigit(char *buffer);
