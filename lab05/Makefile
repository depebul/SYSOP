CC = gcc
CFLAGS = -Wall
TARGETS = zadanie1 catcher sender

all: $(TARGETS)

zadanie1: zadanie1.c
	$(CC) $(CFLAGS) zadanie1.c -o zadanie1

catcher: catcher.c
	$(CC) $(CFLAGS) catcher.c -o catcher

sender: sender.c
	$(CC) $(CFLAGS) sender.c -o sender

clean:
	rm -f $(TARGETS)
	rm -r *.dSYM