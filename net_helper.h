#include <sys/socket.h>

typedef struct sockaddr SA;

int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port);
