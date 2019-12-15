#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "csapp.h"
#include "sbuf.h"

typedef struct {
  int init_n;             // 线程初始数量
  int buf_n;              // sbuf缓冲区大小
  int max_n;              // 线程最大数量
  int free_n;             // 空闲的线程数量
  int n;                  // 当前线程数量
  void *(*func)(void *);  // 线程运行时执行的函数指针
  pthread_mutex_t lock;   // 锁
  pthread_cond_t ready;   // 添加线程的条件变量
  sbuf_t *arg_buf;        // 指针运行函数参数数组缓冲区
} thread_pool_t;

void thread_pool_init(thread_pool_t *tp, int init_n, int max_n, int buf_n,
                      void *(*func)(void *));
void *thread_pool_add(thread_pool_t *tp, void *arg);

#endif
