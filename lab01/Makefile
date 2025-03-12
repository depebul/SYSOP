CC = gcc
CFLAGS = -Wall -std=c17

all: countdown

countdown: countdown.c
	$(CC) $(CFLAGS) countdown.c


clean:
	rm -f countdown *.o
	rm -r *.dSYM
	rm -r *.out

.PHONY: all clean