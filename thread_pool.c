#include "thread_pool.h"

static void *manage_thread(void *arg);
static void *init_worker_thread(void *arg);

void thread_pool_init(thread_pool_t *tp, int init_n, int max_n, int buf_n,
                      void *(*func)(void *)) {
  if (init_n <= 0) app_error("init_n should > 0");
  if (max_n < init_n) app_error("max_n should > init_n");
  tp->init_n = init_n;
  tp->max_n = max_n;
  tp->buf_n = buf_n;
  tp->n = 0;
  tp->is_manage = 0;
  tp->free_n = 0;
  tp->func = func;
  pthread_mutex_init(&tp->lock, NULL);
  pthread_cond_init(&tp->ready, NULL);
  tp->arg_buf = (sbuf_t *)Malloc(sizeof(sbuf_t));
  sbuf_init(tp->arg_buf, tp->buf_n);

  int i;
  pthread_t tid;
  for (i = 0; i < tp->init_n; i++) {
    Pthread_create(&tid, NULL, init_worker_thread, (void *)tp);
    tp->n++;
    tp->free_n++;
  }

  Pthread_create(&tid, NULL, manage_thread, (void *)tp);
}

void *thread_pool_add(thread_pool_t *tp, void *arg) {
  sbuf_insert(tp->arg_buf, arg);
  return NULL;
}

static void *init_worker_thread(void *arg) {
  Pthread_detach(Pthread_self());
  thread_pool_t *tp = (thread_pool_t *)arg;
  void *func_arg;

  while (1) {
    func_arg = sbuf_remove(tp->arg_buf);

    pthread_mutex_lock(&tp->lock);
    tp->free_n--;
    pthread_mutex_unlock(&tp->lock);

    // 需要扩展线程
    if (tp->free_n == 0 && tp->max_n > tp->n && !tp->is_manage) {
      printf("not have enough free thread, begin signal manage_thread\n");
      pthread_cond_signal(&tp->ready);
    }

    // 执行真正的线程任务
    (*tp->func)(func_arg);

    pthread_mutex_lock(&tp->lock);
    tp->free_n++;
    pthread_mutex_unlock(&tp->lock);

    // 需要释放多余线程
    if (tp->free_n > tp->init_n * 2 && !tp->is_manage) {
      printf("have too many worker thread, begin free current thread\n");

      pthread_mutex_lock(&tp->lock);
      tp->n--;
      tp->free_n--;
      pthread_mutex_unlock(&tp->lock);

      return NULL;
    }
  }

  return NULL;
}

// 管理线程 - 添加线程
static void *manage_thread(void *arg) {
  Pthread_detach(Pthread_self());
  thread_pool_t *tp = (thread_pool_t *)arg;
  pthread_t tid;
  int add_n, i;

  while (1) {
    pthread_mutex_lock(&tp->lock);
    while (tp->free_n > 0 || tp->max_n == tp->n || tp->is_manage)
      pthread_cond_wait(&tp->ready, &tp->lock);
    tp->is_manage = 1;
    pthread_mutex_unlock(&tp->lock);

    printf("manage_thread add thread\n");
    add_n = tp->max_n - tp->n > tp->init_n ? tp->init_n : tp->max_n - tp->n;
    for (i = 0; i < add_n; i++) {
      Pthread_create(&tid, NULL, init_worker_thread, arg);
      pthread_mutex_lock(&tp->lock);
      tp->n++;
      tp->free_n++;
      pthread_mutex_unlock(&tp->lock);
    }

    printf("after manage_thread max_n: %d, n: %d, free_n: %d\n", tp->max_n,
           tp->n, tp->free_n);

    pthread_mutex_lock(&tp->lock);
    tp->is_manage = 0;
    pthread_mutex_unlock(&tp->lock);
  }

  return NULL;
}
