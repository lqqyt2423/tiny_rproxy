CC = gcc

CFLAGS = -Og -Wall
LDLIBS = -lpthread -lm

tiny_rproxy: tiny_rproxy.c tiny_rproxy.h csapp.o helper.o sbuf.o sem.o thread_pool.o
	$(CC) $(CFLAGS) -o tiny_rproxy tiny_rproxy.c csapp.o helper.o sbuf.o sem.o thread_pool.o

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

helper.o: helper.c helper.h csapp.h
	$(CC) $(CFLAGS) -c helper.c

sbuf.o: sbuf.c sbuf.h csapp.c csapp.h
	$(CC) $(CFLAGS) -c sbuf.c

sem.o: sem.c sem.h
	$(CC) $(CFLAGS) -c sem.c

thread_pool.o: thread_pool.c thread_pool.h csapp.c csapp.h
	$(CC) $(CFLAGS) -c thread_pool.c

clean:
	rm tiny_rproxy *.o
