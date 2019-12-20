#include "tiny_rproxy.h"

// 一个线程接收请求，一个线程转发请求

typedef struct {
  int connfd;
  int clientfd;
  struct rk_sema mutex; // 用于同步
} proxy_t;

// 客户端请求线程池
thread_pool_t ctp;

static void *handle_client_conn(void *vargp);

// 初始化
void *handle_conn_pool_init(void *arg) {
  thread_pool_init(&ctp, INIT_THREAD, MAX_THREAD, THREAD_BUFSIZE, handle_client_conn);
}

void *handle_conn_pool(void *arg) {
  int connfd = (int)arg;
  int clientfd = open_clientfd(PROXY_HOSTNAME, PROXY_PORT);
  if (clientfd < 0) {
    print_unix_error("open_clientfd error");
    Close(connfd);
    return NULL;
  }

  pthread_t tid;
  int n;
  char buf[MAXBUF];

  proxy_t *pt = (proxy_t *)Malloc(sizeof(proxy_t));
  pt->connfd = connfd;
  pt->clientfd = clientfd;
  rk_sema_init(&pt->mutex, 0);

  thread_pool_add(&ctp, pt);

  while ((n = Rio_read_one(connfd, buf, MAXBUF)) > 0) {
    Rio_writen(clientfd, buf, n);
  }
  Shutdown(clientfd, SHUT_WR);

  rk_sema_wait(&pt->mutex);

  Close(connfd);
  Close(clientfd);
  Free((void *)pt);
  printf("free connfd: %d, clientfd: %d\n", connfd, clientfd);

  return NULL;
}

static void *handle_client_conn(void *vargp) {
  proxy_t *pt = (proxy_t *)vargp;
  int n;
  char buf[MAXBUF];

  while ((n = Rio_read_one(pt->clientfd, buf, MAXBUF)) > 0) {
    Rio_writen(pt->connfd, buf, n);
  }
  Shutdown(pt->connfd, SHUT_WR);

  rk_sema_post(&pt->mutex);
  return NULL;
}
