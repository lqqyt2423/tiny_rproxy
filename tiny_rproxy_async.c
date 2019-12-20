#include "tiny_rproxy.h"

// 多路复用

static void mult_proxy(int connfd);

// do nothing
void *handle_conn_pool_init(void *arg) {}

void *handle_conn_pool(void *arg) {
  int connfd = (int)arg;
  printf("thread get connfd: %d\n", connfd);
  mult_proxy(connfd);
  return NULL;
}

static void mult_proxy(int connfd) {
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
        FD_CLR(connfd, &read_set);
        Shutdown(clientfd, SHUT_WR);
      }
    }
    if (FD_ISSET(clientfd, &ready_set)) {
      if ((n = Rio_read_one(clientfd, buf, MAXBUF)) > 0) {
        Rio_writen(connfd, buf, n);
      } else {
        FD_CLR(clientfd, &read_set);
        Shutdown(connfd, SHUT_WR);
      }
    }
  }
}
