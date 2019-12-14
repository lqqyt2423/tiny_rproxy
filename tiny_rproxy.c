#include "csapp.h"
#include "helper.h"
#include "sbuf.h"

#define NTHREADS 128
#define SBUFSIZE 128
#define BUFSIZE 1024
#define MAXSTR 1024

#define LISTEN_PORT "5432"
#define PROXY_HOSTNAME "localhost"
#define PROXY_PORT "7000"

void *handle_conn(void *vargp);
void *handle_client_conn(void *vargp);
void proxy(int);
void mult_proxy(int);

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

  listenfd = Open_listenfd(LISTEN_PORT);
  printf("listend at %s listenfd: %d\n", LISTEN_PORT, listenfd);

  sbuf_init(&s_buf, SBUFSIZE);

  for (int i = 0; i < NTHREADS; i++) {
    Pthread_create(&tid, NULL, handle_conn, NULL);
  }

  while (1) {
    clientlen = sizeof(struct sockaddr_storage);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXSTR,
                client_port, MAXSTR, 0);
    printf("accept %s:%s connfd: %d\n", client_hostname, client_port, connfd);

    sbuf_insert(&s_buf, connfd);
  }
}

void *handle_conn(void *vargp) {
  Pthread_detach(Pthread_self());
  while (1) {
    int connfd = sbuf_remove(&s_buf);
    printf("thread get connfd: %d\n", connfd);
    mult_proxy(connfd);
  }

  return NULL;
}

// 多路复用
void mult_proxy(int connfd) {
  int clientfd = open_clientfd(PROXY_HOSTNAME, PROXY_PORT);
  if (clientfd < 0) {
    print_unix_error("open_clientfd error");
    Close(connfd);
    return;
  }

  int n;
  int maxfd = clientfd > connfd ? clientfd : connfd;
  char buf[MAXBUF];
  fd_set read_set, ready_set;
  FD_ZERO(&read_set);
  FD_SET(connfd, &read_set);
  FD_SET(clientfd, &read_set);
  while (1) {
    if (!FD_ISSET(connfd, &read_set) && !FD_ISSET(clientfd, &read_set)) {
      Close(connfd);
      Close(clientfd);
      printf("closed connfd %d and clientfd %d\n", connfd, clientfd);
      break;
    }
    ready_set = read_set;
    Select(maxfd + 1, &ready_set, NULL, NULL, NULL);
    if (FD_ISSET(connfd, &ready_set)) {
      if ((n = Rio_read_one(connfd, buf, MAXBUF)) > 0) {
        Rio_writen(clientfd, buf, n);
      } else {
        shutdown(connfd, SHUT_RD);
        shutdown(clientfd, SHUT_WR);
        FD_CLR(connfd, &read_set);
      }
    }
    if (FD_ISSET(clientfd, &ready_set)) {
      if ((n = Rio_read_one(clientfd, buf, MAXBUF)) > 0) {
        Rio_writen(connfd, buf, n);
      } else {
        shutdown(clientfd, SHUT_RD);
        shutdown(connfd, SHUT_WR);
        FD_CLR(clientfd, &read_set);
      }
    }
  }
}

// 多线程
void proxy(int connfd) {
  int clientfd = open_clientfd(PROXY_HOSTNAME, PROXY_PORT);
  if (clientfd < 0) {
    print_unix_error("open_clientfd error");
    Close(connfd);
    return;
  }

  pthread_t tid;
  int n;
  char buf[MAXBUF];

  proxy_t *pt = (proxy_t *)Malloc(sizeof(proxy_t));
  pt->connfd = connfd;
  pt->clientfd = clientfd;
  Pthread_create(&tid, NULL, handle_client_conn, pt);

  while ((n = Rio_read_one(connfd, buf, MAXBUF)) > 0) {
    Rio_writen(clientfd, buf, n);
  }

  shutdown(connfd, SHUT_RD);
  shutdown(clientfd, SHUT_WR);

  Pthread_join(tid, NULL);

  Close(connfd);
  Close(clientfd);
  Free((void *)pt);
  printf("free connfd: %d, clientfd: %d\n", connfd, clientfd);
}

void *handle_client_conn(void *vargp) {
  proxy_t *pt = (proxy_t *)vargp;
  int n;
  char buf[MAXBUF];

  while ((n = Rio_read_one(pt->clientfd, buf, MAXBUF)) > 0) {
    Rio_writen(pt->connfd, buf, n);
  }
  shutdown(pt->clientfd, SHUT_RD);
  shutdown(pt->connfd, SHUT_WR);

  return NULL;
}
