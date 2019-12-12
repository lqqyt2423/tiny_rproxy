#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include "net_helper.h"
#include "rio.h"
#include "sbuf.h"

#define NTHREADS 4
#define SBUFSIZE 16
#define BUFSIZE 1024
#define MAXSTR 100

#define LISTEN_PORT "5432"
#define PROXY_HOSTNAME "localhost"
#define PROXY_PORT "7000"

void *handle_conn(void *vargp);
void *handle_client_conn(void *vargp);
void proxy(int);

typedef struct {
  int connfd;
  int clientfd;
} proxy_t;

sbuf_t s_buf;

int main(int argc, char **argv) {
  int listenfd, connfd;
  pthread_t tid;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  char client_hostname[MAXSTR], client_port[MAXSTR];

  listenfd = open_listenfd(LISTEN_PORT);
  printf("listend at %s listenfd: %d\n", LISTEN_PORT, listenfd);

  sbuf_init(&s_buf, SBUFSIZE);

  for (int i = 0; i < NTHREADS; i++) {
    pthread_create(&tid, NULL, handle_conn, NULL);
    printf("create pthread: %d\n", i);
  }

  while (1) {
    clientlen = sizeof(struct sockaddr_storage);
    connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
    getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXSTR,
                client_port, MAXSTR, 0);
    printf("accept %s:%s connfd: %d\n", client_hostname, client_port, connfd);

    sbuf_insert(&s_buf, connfd);
  }
}

void *handle_conn(void *vargp) {
  pthread_detach(pthread_self());
  while (1) {
    int connfd = sbuf_remove(&s_buf);
    printf("thread get connfd: %d\n", connfd);
    proxy(connfd);
  }

  return NULL;
}

void proxy(int connfd) {
  int clientfd = open_clientfd(PROXY_HOSTNAME, PROXY_PORT);
  if (clientfd == -1) {
    fprintf(stderr, "server %s:%s error\n", PROXY_HOSTNAME, PROXY_PORT);
    return;
  }
  printf("open proxy %s:%s clientfd: %d\n", PROXY_HOSTNAME, PROXY_PORT,
         clientfd);

  pthread_t tid;
  int n;
  char buf[BUFSIZE];

  proxy_t *pt = (proxy_t *)malloc(sizeof(proxy_t));
  pt->connfd = connfd;
  pt->clientfd = clientfd;
  pthread_create(&tid, NULL, handle_client_conn, pt);

  while ((n = rio_readn(connfd, buf, BUFSIZE)) > 0) {
    rio_writen(clientfd, buf, n);
  }

  pthread_join(tid, NULL);

  close(connfd);
  close(clientfd);
  free((void *)pt);
}

void *handle_client_conn(void *vargp) {
  proxy_t *pt = (proxy_t *)vargp;
  int n;
  char buf[BUFSIZE];

  while ((n = rio_readn(pt->clientfd, buf, BUFSIZE)) > 0) {
    rio_writen(pt->connfd, buf, n);
  }

  return NULL;
}
