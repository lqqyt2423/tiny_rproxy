#include "tiny_rproxy.h"

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
