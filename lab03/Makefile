CC = gcc
CFLAGS = -Wall -std=c17 -g

all: flipper

flipper: flipper.c
	$(CC) $(CFLAGS) -c flipper.c
	$(CC) $(CFLAGS) flipper.c -o flipper


clean:
	rm -f flipper *.o
	rm -r *.dSYM

.PHONY: all clean