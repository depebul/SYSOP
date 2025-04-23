CFLAGS = -Wall
CC = gcc

sender: sender.o

receiver: receiver.o

clean:
	rm -f sender.o sender receiver.o receiver squareFIFO

test: sender receiver 
	@echo -e "\nTESTING\n"
	./sender 22 &
	./receiver
	wait
	./sender 9 &
	./receiver
	wait
	./sender -13 &
	./receiver
	wait
	./sender cos &
	./receiver
	wait
all: sender receiver
