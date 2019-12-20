#include "tiny_rproxy.h"

int main(int argc, char **argv) {
  int listenfd, connfd;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  char client_hostname[MAXSTR], client_port[MAXSTR];
  int flags = NI_NUMERICHOST | NI_NUMERICSERV;

  listenfd = Open_listenfd(LISTEN_PORT);
  printf("listend at %s listenfd: %d\n", LISTEN_PORT, listenfd);

  // 初始化线程池
  thread_pool_t tp;
  thread_pool_init(&tp, INIT_THREAD, MAX_THREAD, THREAD_BUFSIZE, handle_conn_pool);

  // 初始化客户端线程池 或 do nothing
  handle_conn_pool_init(NULL);

  while (1) {
    clientlen = sizeof(struct sockaddr_storage);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXSTR,
                client_port, MAXSTR, flags);
    printf("accept %s:%s connfd: %d\n", client_hostname, client_port, connfd);
    thread_pool_add(&tp, (void *)connfd);
  }
}
