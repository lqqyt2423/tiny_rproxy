#include "sbuf.h"

void sbuf_init(sbuf_t *sp, int n) {
  sp->buf = Calloc(n, sizeof(void *));
  sp->n = n;
  sp->front = sp->rear = 0;

  rk_sema_init(&sp->mutex, 1);
  rk_sema_init(&sp->slots, n);
  rk_sema_init(&sp->items, 0);
}

void sbuf_deinit(sbuf_t *sp) { Free(sp->buf); }

void sbuf_insert(sbuf_t *sp, void *item) {
  rk_sema_wait(&sp->slots);
  rk_sema_wait(&sp->mutex);
  sp->buf[(++sp->rear) % (sp->n)] = item;
  rk_sema_post(&sp->mutex);
  rk_sema_post(&sp->items);
}

void *sbuf_remove(sbuf_t *sp) {
  void *item;
  rk_sema_wait(&sp->items);
  rk_sema_wait(&sp->mutex);
  item = sp->buf[(++sp->front) % (sp->n)];
  rk_sema_post(&sp->mutex);
  rk_sema_post(&sp->slots);
  return item;
}
