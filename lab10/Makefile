CC=gcc
CFLAGS=-Wall -pthread

all: zad

zad: main.c 
	$(CC) $(CFLAGS) main.c -o main

.PHONY: clean
clean:
	rm main server