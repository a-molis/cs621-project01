#include <stddef.h>

#ifndef TCP_SOCK_HANDLER_H_
#define TCP_SOCK_HANDLER_H_

struct TCP_SOCKET_HANDLER 
{
  int sockfd;
};

typedef struct TCP_SOCKET_HANDLER *TCP_HANDLER;

TCP_HANDLER new_tcp_handler(int socket_fd);
int tcp_sendn(TCP_HANDLER handler, char *buf, int buf_len);
int tcp_recvn(TCP_HANDLER handler, char *buf, int buf_len);
int tcp_send(TCP_HANDLER handler, char *buf, int buf_len);
int tcp_recv(TCP_HANDLER handler, char **buf, int buf_len);
int destroy_tcp_handler(TCP_HANDLER handler);

#endif
