#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "tcp_sock_handler.h"


TCP_HANDLER new_tcp_handler(int sockfd) 
{
  TCP_HANDLER handler = malloc(sizeof(struct TCP_SOCKET_HANDLER));
  handler->sockfd = sockfd;
  return handler; 
}

size_t tcp_recvn(TCP_HANDLER handler, char *buf, size_t buf_len)
{
  size_t received = recv(handler->sockfd, buf, buf_len, 0);
  if (received < 0)
    {
      perror("tcp_recvn failed to recv enough data from socket");
      return 1;
    }
  else if (received == 0)
    return EOF;
  else
    return received;
}

size_t tcp_sendn(TCP_HANDLER handler, char *buf, size_t buf_len)
{
  size_t sent = send(handler->sockfd, buf, buf_len, 0);
  if (sent < 0)
    {
      perror("Error with tcp_sendn failed to send enough data");
      return 1;
    }
  return 0;
}

size_t tcp_send(TCP_HANDLER handler, char *buf, size_t buf_len)
{
  char num_buf[4];
  uint32_t len_nb = htonl(buf_len);
  for ( int i = 3, shift = 0; i > 0; i--, shift++) {
    // num_buf[3] = (len_nb >> 0) & 0xFF;
    num_buf[i] = (len_nb >> shift) & 0xFF;
  }
  size_t sent_size = tcp_sendn(handler, num_buf, 4);
  if (sent_size) 
    {
      perror("Error with tcp_send failed to sent size");
      return 1;
    }
  size_t sent_data = tcp_sendn(handler, buf, buf_len);
  if (sent_data)
    {
      perror("Error sending data with tcp_send");
      return 1;
    }
  return 0;
}

