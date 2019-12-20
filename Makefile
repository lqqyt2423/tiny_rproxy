CC = gcc

CFLAGS = -Og -Wall
LDLIBS = -pthread

all: tiny_rproxy_mult.out tiny_rproxy_async.out

tiny_rproxy_mult.out: tiny_rproxy.c tiny_rproxy.h tiny_rproxy_mult.o csapp.o helper.o sbuf.o sem.o thread_pool.o
	$(CC) $(CFLAGS) $(LDLIBS) -o tiny_rproxy_mult.out tiny_rproxy.c tiny_rproxy_mult.o csapp.o helper.o sbuf.o sem.o thread_pool.o

tiny_rproxy_async.out: tiny_rproxy.c tiny_rproxy.h tiny_rproxy_async.o csapp.o helper.o sbuf.o sem.o thread_pool.o
	$(CC) $(CFLAGS) $(LDLIBS) -o tiny_rproxy_async.out tiny_rproxy.c tiny_rproxy_async.o csapp.o helper.o sbuf.o sem.o thread_pool.o

tiny_rproxy_mult.o: tiny_rproxy_mult.c
	$(CC) $(CFLAGS) -c tiny_rproxy_mult.c

tiny_rproxy_async.o: tiny_rproxy_async.c
	$(CC) $(CFLAGS) -c tiny_rproxy_async.c

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
	rm *.out *.o
