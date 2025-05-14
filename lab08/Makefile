CC = gcc
CFLAGS = -Wall
TARGETS =  client printer init 

all: $(TARGETS)

client: client.c
	$(CC) $(CFLAGS) client.c -o client

printer: printer.c
	$(CC) $(CFLAGS) printer.c -o printer

init: init.c
	$(CC) $(CFLAGS) init.c -o init

clean:
	rm -f $(TARGETS)
	rm -rf *.dSYM