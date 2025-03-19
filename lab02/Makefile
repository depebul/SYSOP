CC = gcc
CFLAGS = -Wall -std=c17

all: static shared client_static client_shared client_dynamic


static: collatz.o
		ar rcs libcollatz.a collatz.o


collatz.o: collatz.c
		$(CC) $(CFLAGS) -c collatz.c


shared: collatz.so


collatz.so: collatz.c
		$(CC) $(CFLAGS) -fPIC -shared -o libcollatz.so collatz.c


client_static: main.c libcollatz.a
		$(CC) $(CFLAGS) -o client_static main.c -L. -lcollatz


client_shared: main.c libcollatz.so
		$(CC) $(CFLAGS) -o client_shared main.c -L. -lcollatz


client_dynamic: main.c
		$(CC) $(CFLAGS) -o client_dynamic main.c -ldl -DUSE_DYNAMIC


clean:
		rm -f *.o *.a *.so client_static client_shared client_dynamic