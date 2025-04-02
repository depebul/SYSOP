CC = gcc
CFLAGS = -Wall -std=c17

all: zad1 zad2

zad1: zad1.c
		$(CC) $(CFLAGS) zad1.c -o zad1

zad2: zad2.c
		$(CC) $(CFLAGS) zad2.c -o zad2

clean:
		rm -f zad1 zad2
		rm -r *.dSYM

.PHONY: all clean