CC = gcc

CFLAGS = -Og -Wall
LDLIBS = -lpthread -lm

tiny_rproxy: tiny_rproxy.c csapp.o helper.o sbuf.o sem.o

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

helper.o: helper.c helper.h csapp.h
	$(CC) $(CFLAGS) -c helper.c

sbuf.o: sbuf.c sbuf.h csapp.c csapp.h
	$(CC) $(CFLAGS) -c sbuf.c csapp.c

sem.o: sem.c sem.h
	$(CC) $(CFLAGS) -c sem.c

clean:
	rm tiny_rproxy *.o
