#include "helper.h"

void print_unix_error(char *msg) {
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}

ssize_t rio_read_one(int fd, void *usrbuf, size_t n) {
  ssize_t nread;
  while (1) {
    if ((nread = read(fd, usrbuf, n)) < 0) {
      if (errno == EINTR) {
        continue;
      } else {
        return -1;
      }
    } else if (nread == 0) {
      return 0;
    } else {
      return nread;
    }
  }
}

ssize_t Rio_read_one(int fd, void *usrbuf, size_t n) {
  ssize_t nread;

  if ((nread = rio_read_one(fd, usrbuf, n)) < 0)
    print_unix_error("Rio_read_one error");
  return nread;
}
