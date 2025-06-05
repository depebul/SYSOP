CC=gcc
CFLAGS=-Wall -pthread

all: zad

zad: client.c server.c
	$(CC) $(CFLAGS) server.c -o server
	$(CC) $(CFLAGS) client.c -o client

.PHONY: clean
clean:
	rm client server