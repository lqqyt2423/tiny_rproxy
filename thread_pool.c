#include "thread_pool.h"

static void *add_thread(void *arg);
static void *init_single_thread(void *arg);

void thread_pool_init(thread_pool_t *tp, int init_n, int max_n, int buf_n,
                      void *(*func)(void *)) {
  if (init_n <= 0) app_error("init_n should > 0");
  if (max_n < init_n) app_error("max_n should > init_n");
  tp->init_n = init_n;
  tp->max_n = max_n;
  tp->buf_n = buf_n;
  tp->n = 0;
  tp->free_n = 0;
  tp->func = func;
  pthread_mutex_init(&tp->lock, NULL);
  pthread_cond_init(&tp->ready, NULL);
  tp->arg_buf = (sbuf_t *)Malloc(sizeof(sbuf_t));
  sbuf_init(tp->arg_buf, tp->buf_n);

  int i;
  pthread_t tid;
  Pthread_create(&tid, NULL, add_thread, (void *)tp);
  for (i = 0; i < tp->init_n; i++) {
    Pthread_create(&tid, NULL, init_single_thread, (void *)tp);
    tp->n++;
    tp->free_n++;
  }
}

void *thread_pool_add(thread_pool_t *tp, void *arg) {
  sbuf_insert(tp->arg_buf, arg);

  pthread_mutex_lock(&tp->lock);
  tp->free_n--;
  if (tp->free_n == 0 && tp->max_n > tp->n) {
    printf("not have enough free thread, begin signal add_thread\n");
    pthread_cond_signal(&tp->ready);
  }
  pthread_mutex_unlock(&tp->lock);

  return NULL;
}

static void *init_single_thread(void *arg) {
  Pthread_detach(Pthread_self());
  thread_pool_t *tp = (thread_pool_t *)arg;
  void *func_arg;

  while (1) {
    func_arg = sbuf_remove(tp->arg_buf);
    (*tp->func)(func_arg);

    pthread_mutex_lock(&tp->lock);
    tp->free_n++;
    pthread_mutex_unlock(&tp->lock);
  }

  return NULL;
}

// 管理线程 - 添加线程
static void *add_thread(void *arg) {
  Pthread_detach(Pthread_self());
  thread_pool_t *tp = (thread_pool_t *)arg;
  pthread_t tid;

  while (1) {
    pthread_mutex_lock(&tp->lock);
    while (tp->free_n > 0) pthread_cond_wait(&tp->ready, &tp->lock);
    pthread_mutex_unlock(&tp->lock);

    if (tp->max_n == tp->n) {
      continue;
    }

    printf("add_thread add thread\n");
    Pthread_create(&tid, NULL, init_single_thread, arg);

    pthread_mutex_lock(&tp->lock);
    tp->n++;
    tp->free_n++;
    pthread_mutex_unlock(&tp->lock);

    printf("after add_thread max_n: %d, n: %d, free_n: %d\n", tp->max_n, tp->n,
           tp->free_n);
  }

  return NULL;
}
