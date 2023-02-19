#include <stddef.h>

#ifndef TCP_SOCK_HANDLER_H_
#define TCP_SOCK_HANDLER_H_

struct TCP_SOCKET_HANDLER 
{
  int sockfd;
};

typedef struct TCP_SOCKET_HANDLER *TCP_HANDLER;

TCP_HANDLER new_tcp_handler(int socket_fd);
size_t tcp_sendn(TCP_HANDLER handler, char *buf, size_t buf_len);
size_t tcp_recvn(TCP_HANDLER handler, char *buf, size_t buf_len);
size_t tcp_send(TCP_HANDLER handler, char *buf, size_t buf_len);
size_t tcp_recv(TCP_HANDLER handler, char *buf, size_t buf_len);

#endif
