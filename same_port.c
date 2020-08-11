#include "tiny_rproxy.h"

// 父与子监听同一个 listenfd

void sigint_handler(int sig) { printf("caught sigint\n"); }

int main(int argc, char **argv) {
  printf("argc: %d, argv: %s\n", argc, argv[0]);
  if (argc > 1) {
    int listenfd = atoi(argv[1]);
    if (listenfd <= 2) {
      printf("wrong listenfd: %d\n", listenfd);
      exit(1);
    }

    printf("receive listenfd %d, start accept\n", listenfd);

    socklen_t clientlen;
    int connfd;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXSTR], client_port[MAXSTR];
    int flags = NI_NUMERICHOST | NI_NUMERICSERV;
    char *resp = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nhello world\n";
    size_t respLen = strlen(resp);

    while (1) {
      clientlen = sizeof(struct sockaddr_storage);
      connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
      Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXSTR,
                  client_port, MAXSTR, flags);
      printf("accept %s:%s connfd: %d pid: %d\n", client_hostname, client_port,
             connfd, getpid());
      Rio_writen(connfd, resp, respLen);
      Close(connfd);
    }

    exit(0);
  }

  int listenfd = Open_listenfd(LISTEN_PORT);
  printf("listend at %s listenfd: %d\n", LISTEN_PORT, listenfd);

  pid_t cpid;
  if ((cpid = Fork()) > 0) {
    Signal(SIGINT, sigint_handler);

    printf("parent pid: %d child pid: %d\n", getpid(), cpid);
    // parent
    Waitpid(cpid, NULL, 0);
    printf("parent wait for child exit\n");
  } else {
    // child
    char argv1[10];
    sprintf(argv1, "%d", listenfd);
    char *argv[] = {"same_port.out", argv1};
    Execve(argv[0], &argv[0], NULL);
  }
}
