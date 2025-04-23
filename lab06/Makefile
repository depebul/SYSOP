CC = gcc
CFLAGS = -Wall
TARGETS = zadanie1 client calculator

all: $(TARGETS)

zadanie1: zadanie1.c
	$(CC) $(CFLAGS) zadanie1.c -o zadanie1

client: client.c
	$(CC) $(CFLAGS) client.c -o client

calculator: calculator.c
	$(CC) $(CFLAGS) calculator.c -o calculator

clean:
	rm -f $(TARGETS)
	rm -r *.dSYM