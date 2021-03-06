#include "sem.h"

void rk_sema_init(struct rk_sema *s, int value) {
#ifdef __APPLE__
  dispatch_semaphore_t *sem = &s->sem;

  *sem = dispatch_semaphore_create(value);
#else
  sem_init(&s->sem, 0, value);
#endif
}

void rk_sema_wait(struct rk_sema *s) {
#ifdef __APPLE__
  dispatch_semaphore_wait(s->sem, DISPATCH_TIME_FOREVER);
#else
  int r;

  do {
    r = sem_wait(&s->sem);
  } while (r == -1 && errno == EINTR);
#endif
}

void rk_sema_post(struct rk_sema *s) {
#ifdef __APPLE__
  dispatch_semaphore_signal(s->sem);
#else
  sem_post(&s->sem);
#endif
}
