CC = gcc
CFLAGS = -Wall -pthread
TARGETS = main

all: $(TARGETS)

main: main.c
	$(CC) $(CFLAGS) main.c -o main


clean:
	rm -f $(TARGETS)
	rm -r *.dSYM