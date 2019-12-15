#include "tiny_rproxy.h"

int main(int argc, char **argv) {
  int listenfd, connfd;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  char client_hostname[MAXSTR], client_port[MAXSTR];

  listenfd = Open_listenfd(LISTEN_PORT);
  printf("listend at %s listenfd: %d\n", LISTEN_PORT, listenfd);

  // 初始化线程池
  thread_pool_t tp;
  thread_pool_init(&tp, INIT_THREAD, MAX_THREAD, THREAD_BUFSIZE,
                   handle_conn_pool);

  while (1) {
    clientlen = sizeof(struct sockaddr_storage);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXSTR,
                client_port, MAXSTR, 0);
    printf("accept %s:%s connfd: %d\n", client_hostname, client_port, connfd);
    thread_pool_add(&tp, (void *)connfd);
  }
}

void *handle_conn_pool(void *arg) {
  int connfd = (int)arg;
  printf("thread get connfd: %d\n", connfd);
  mult_proxy(connfd);
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
