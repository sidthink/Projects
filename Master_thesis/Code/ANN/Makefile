BIN = test

CFLAGS = -std=c99 -Wall -Wextra -pedantic -Ofast -flto -march=native

LDFLAGS = -lm

CC = gcc

SRC = neuraltest.c ann.c dataread.c

all:
	$(CC) -o $(BIN) $(SRC) $(CFLAGS) $(LDFLAGS)

run:
	./$(BIN)

clean:
	rm -f $(BIN)