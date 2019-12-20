#ifndef __TINY_RPROXY_H__
#define __TINY_RPROXY_H__

#include "csapp.h"
#include "helper.h"
#include "sbuf.h"
#include "thread_pool.h"

#define INIT_THREAD 8
#define MAX_THREAD 128
#define THREAD_BUFSIZE 128
#define BUFSIZE 1024
#define MAXSTR 1024

#define LISTEN_PORT "5432"
#define PROXY_HOSTNAME "localhost"
#define PROXY_PORT "7000"

void *handle_conn_pool_init(void *arg);
void *handle_conn_pool(void *arg);

#endif
