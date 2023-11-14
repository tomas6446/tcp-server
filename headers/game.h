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

long* initGame();

void resetGame(long *answer, Client *client, char buffer[BUFF_LEN]);

void handleGuess(Client *clients, int client_index, char buffer[BUFF_LEN], long *answer);

long randomize(int lower, int upper);

int isDigit(char *buffer);
