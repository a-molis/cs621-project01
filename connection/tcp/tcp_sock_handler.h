
#ifndef TCP_SOCK_HANDLER_H_
#define TCP_SOCK_HANDLER_H_

struct TCP_SOCKET_HANDLER 
{
  int socket_fd;
};

typedef struct TCP_SOCKET_HANDLER *TCP_HANDLER;

TCP_HANDLER new_tcp_handler(int socket_fd);

#endif
