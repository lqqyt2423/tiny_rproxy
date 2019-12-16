#ifndef __HELPER_H__
#define __HELPER_H__

#include "csapp.h"

void print_unix_error(char *msg);

ssize_t rio_read_one(int fd, void *usrbuf, size_t n);
ssize_t Rio_read_one(int fd, void *usrbuf, size_t n);

void Shutdown(int fd, int s);

#endif
