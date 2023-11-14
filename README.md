# TCP Server-Client Guess the Number Game

This is a simple guessing game that is played between a client and a server.
The server chooses a random number between 1 and 100, and the client has 10 attempts to handle_guess the number.
After each handle_guess, the server responds with either "Higher", "Lower", or "Correct!"
to help the client make the next handle_guess.
If the client guesses the number correctly, they win the game.

## Usage
To compile the program:
```bash
gcc -o server src/server.c src/game.c src/connection.c src/client_management.c -lpthread
gcc -o client src/client.c src/connection.c -lpthread
 ```
### Run the server on port 5000
``` bash
./server 5000
```
### To run the client on localhost and port 5000:
``` bash
./client 127.0.0.1 5000
```

## Options
The following options are available for the client:

    /q - Quit the game
    /m - Message all clients connected to the server

