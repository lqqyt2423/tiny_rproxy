#ifndef __SBUF_H__
#define __SBUF_H__

#include "csapp.h"
#include "sem.h"

typedef struct {
  int *buf;
  int n;
  int front;
  int rear;

  struct rk_sema mutex;
  struct rk_sema slots;
  struct rk_sema items;
} sbuf_t;

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);

#endif
