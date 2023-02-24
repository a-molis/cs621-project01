#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include "tcp_sock_handler.h"

// TODO add free/destroy
TCP_HANDLER new_tcp_handler(int sockfd) 
{
  TCP_HANDLER handler = malloc(sizeof(struct TCP_SOCKET_HANDLER));
  printf("sockfd at  new %d\n", sockfd);
  handler->sockfd = sockfd;
  printf("sockfd at assignment %d\n", handler->sockfd);
  return handler; 
}

int destroy_tcp_handler(TCP_HANDLER handler)
{
  if (handler) 
    free (handler);
  return 0;
}

int tcp_recvn(TCP_HANDLER handler, char *buf, int buf_len)
{
  printf("Trying to receive data with len %d\n", buf_len);
  int total = 0;
  while (total < buf_len) 
    {
      printf("total %d\n", total);
      int received = recv(handler->sockfd, buf + total, buf_len, 0);
      if (received < 0)
        {
          perror("tcp_recvn failed to recv enough data from socket");
          return 1;
        }
       total += received;
       printf("Total in recivn: %d\n", total);
    }
    return total;
}

int tcp_sendn(TCP_HANDLER handler, char *buf, int buf_len)
{
  printf("Sending with size %d\n", buf_len);
  int total = 0;
  printf("total: %d,  len: %d\n", total, buf_len);
  while (total < buf_len)
    {
      printf("Sockfd in sendn %d\n", handler->sockfd);
      printf("sending data %s\n", buf + total);
      int sent = send(handler->sockfd, buf + total, buf_len, 0);
      total += sent;
      printf("total sent in sendn %d\n", total);
    }
  printf("total: %d, buf_len: %d, total < buf_len: %d\n", total, buf_len, total < buf_len); 
  return 0;
}

int tcp_send(TCP_HANDLER handler, char *buf, int buf_len)
{
  char num_buf[4];
  uint32_t len_nb = htonl(buf_len);
  num_buf[3] = (len_nb >> 0) & 0xFF;
  num_buf[2] = (len_nb >> 8) & 0xFF;
  num_buf[1] = (len_nb >> 16) & 0xFF;
  num_buf[0] = (len_nb >> 24) & 0xFF;
  int sent_size = tcp_sendn(handler, num_buf, 4);
  if (sent_size) 
    {
      perror("Error with tcp_send failed to sent size");
      return 1;
    }
  int sent_data = tcp_sendn(handler, buf, buf_len);
  if (sent_data)
    {
      perror("Error sending data with tcp_send");
      return 1;
    }
  return 0;
}

int tcp_recv(TCP_HANDLER handler, char **buf, int buf_len)
{
  char num_buf[4];
  int recv_len = tcp_recvn(handler, num_buf, 4); 
  if (recv_len < 0)
    {
      perror("tcp_recvn failed to recv enough data\
          from socket when getting length");
      return 1;
    }
  else if (recv_len == 0)
    return EOF; 
  uint32_t len_nb = (num_buf[0] << 24)
                    || (num_buf[1] << 16) 
                    || (num_buf[2] << 8)
                    || (num_buf[3] << 0);
  int size = ntohl(len_nb);
  *buf = malloc(sizeof(char) * size); 
  int received = tcp_recvn(handler, *buf, size); 
  if (received < 0)
    {
      perror("tcp_recv failed to recv enough data \
          from socket when getting data");
      return 1;
    }
  else if (recv_len == 0)
    return EOF; 
  return 0;
}



