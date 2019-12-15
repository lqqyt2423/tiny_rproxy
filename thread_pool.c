#include "thread_pool.h"

static void *init_single_thread(void *arg);

void thread_pool_init(thread_pool_t *tp, int init_n, int max_n,
                      void *(*func)(void *)) {
  if (init_n <= 0) app_error("init_n should > 0");
  if (max_n < init_n) app_error("max_n should > init_n");
  tp->init_n = init_n;
  tp->max_n = max_n;
  tp->n = 0;
  tp->free_n = 0;
  tp->func = func;
  pthread_mutex_init(&tp->lock, NULL);
  pthread_cond_init(&tp->ready, NULL);

  int i;
  pthread_t tid;
  for (i = 0; i < tp->init_n; i++) {
    Pthread_create(&tid, NULL, init_single_thread, (void *)tp);
    tp->n++;
    tp->free_n++;
  }

  // 初始化指向参数指针的存储空间
  // TODO: 动态分配
  tp->args = Calloc(tp->max_n, sizeof(void *));
}

// 将参数指针放入第一个为空的参数指针数组中，然后发出信号，让其他线程来读取
void *thread_pool_add(thread_pool_t *tp, void *arg) {
  pthread_mutex_lock(&tp->lock);
  int i;

  if (tp->free_n > 0) {
    for (i = 0; i < tp->n; i++) {
      if (tp->args[i] == NULL) {
        tp->args[i] = arg;
        tp->free_n--;
        pthread_cond_signal(&tp->ready);
        break;
      }
    }
  } else {
    if (tp->n == tp->max_n) {
      printf("not enough free_n, but reach max_n, can't scale\n");
    } else {
      printf("not enough free_n, begin scale\n");
      pthread_t tid;
      Pthread_create(&tid, NULL, init_single_thread, (void *)tp);
      tp->args[tp->n++] = arg;
      pthread_cond_signal(&tp->ready);
    }
  }

  pthread_mutex_unlock(&tp->lock);

  return NULL;
}

static void *init_single_thread(void *arg) {
  Pthread_detach(Pthread_self());
  thread_pool_t *tp = (thread_pool_t *)arg;
  void *func_arg;

  while (1) {
    pthread_mutex_lock(&tp->lock);
    while (tp->free_n == tp->n) pthread_cond_wait(&tp->ready, &tp->lock);

    // 尝试取待执行方法的参数了
    int i;
    func_arg = NULL;
    for (i = 0; i < tp->n; i++) {
      if (tp->args[i] != NULL) {
        func_arg = tp->args[i];
        tp->free_n++;
        tp->args[i] = NULL;
        break;
      }
    }

    pthread_mutex_unlock(&tp->lock);

    // 真正开始执行函数
    if (func_arg != NULL) (*tp->func)(func_arg);
  }

  return NULL;
}
