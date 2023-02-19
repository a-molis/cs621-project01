#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "tcp_sock_handler.h"


TCP_HANDLER new_tcp_handler(int socket_fd) 
{
  TCP_HANDLER handler = malloc(sizeof(struct TCP_SOCKET_HANDLER));
  handler->socket_fd = socket_fd;
  return handler; 
}
