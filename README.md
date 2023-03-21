# TCP Server-Client Guess the Number Game

This is a simple guessing game that is played between a client and a server. 
The server chooses a random number between 1 and 100, and the client has 10 attempts to guess the number. 
After each guess, the server responds with either "Higher", "Lower", or "Correct!" 
to help the client make the next guess. 
If the client guesses the number correctly, they win the game.

## Usage
To compile the program:
``` bash
gcc -o server server.c
gcc -o client client.c
```


To start the server:
``` bash
./server <server-port>
```
To start a client and connect to the server:
``` bash
./client <server-ip> <server-port>
```

## Options

The following options are available for the client:

    /q - Quit the game
    /m - Message all clients connected to the server

