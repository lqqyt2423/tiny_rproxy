#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include "net_helper.h"
#include "rio.h"
#include "sbuf.h"
#include <stdlib.h>

#define NTHREADS 4
#define SBUFSIZE 16
#define BUFSIZE 1024

#define LISTEN_PORT "5432"
#define PROXY_HOSTNAME "localhost"
#define PROXY_PORT "7000"

void *handle_conn(void *vargp);
void *handle_client_conn(void *vargp);
void proxy(int);

sbuf_t s_buf;

typedef struct {
  int connfd;
  int clientfd;
} proxy_t;

int main(int argc, char **argv) {
  int listenfd, connfd;
  pthread_t tid;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  listenfd = open_listenfd(LISTEN_PORT);
  sbuf_init(&s_buf, SBUFSIZE);

  for (int i = 0; i < NTHREADS; i++) {
    pthread_create(&tid, NULL, handle_conn, NULL);
  }

  while (1) {
    clientlen = sizeof(struct sockaddr_storage);
    connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
    sbuf_insert(&s_buf, connfd);
  }
}

void *handle_conn(void *vargp) {
  pthread_detach(pthread_self());
  while (1) {
    int connfd = sbuf_remove(&s_buf);
    proxy(connfd);
  }

  return NULL;
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

void proxy(int connfd) {
  int clientfd = open_clientfd(PROXY_HOSTNAME, PROXY_PORT);
  if (clientfd == -1) {
    fprintf(stderr, "server %s:%s error\n", PROXY_HOSTNAME, PROXY_PORT);
    return;
  }

  pthread_t tid;
  int up_n, down_n;
  char up_buf[BUFSIZE], down_buf[BUFSIZE];

  proxy_t *pt = (proxy_t *)malloc(sizeof(proxy_t));
  pt->connfd = connfd;
  pt->clientfd = clientfd;
  pthread_create(&tid, NULL, handle_client_conn, pt);

  while ((up_n = rio_readn(connfd, up_buf, BUFSIZE)) > 0) {
    rio_writen(clientfd, down_buf, up_n);
  }

  pthread_join(tid, NULL);

  close(connfd);
  close(clientfd);
  free((void *)pt);
}
