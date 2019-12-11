#include <pthread.h>
#include <stdio.h>
#include "net_helper.h"
#include "rio.h"
#include "sbuf.h"

#define NTHREADS 4
#define SBUFSIZE 16
#define BUFSIZE 1024

#define LISTEN_PORT "8000"
#define PROXY_HOSTNAME "localhost"
#define PROXY_PORT "8001"

void *handle_conn(void *vargp);
void proxy(int);

sbuf_t s_buf;

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
}

void proxy(int connfd) {
  int clientfd = open_clientfd(PROXY_HOSTNAME, PROXY_PORT);
  if (clientfd == -1) {
    fprintf(stderr, "server %s:%s error\n", PROXY_HOSTNAME, PROXY_PORT);
    return;
  }

  rio_t up, down;
  int up_n, down_n;
  char up_buf[BUFSIZE], down_buf[BUFSIZE];
  rio_readinitb(&up, connfd);
  rio_readinitb(&down, clientfd);

  while (1) {
    up_n = rio_readnb(&up, up_buf, BUFSIZE);
    if (up_n > 0) {
      rio_writen(clientfd, up_buf, up_n);
    }
    down_n = rio_readnb(&down, down_buf, BUFSIZE);
    if (down_n > 0) {
      rio_writen(connfd, down_buf, down_n);
    }
    if (up_n <= 0 && down_n <= 0) {
      close(clientfd);
      close(connfd);
      return;
    }
  }
}
