/**
 * This file include wrappers for sockets and functions around sending and receiving data with TCP.
 */
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tcp_sock_handler.h"

TCP_HANDLER
new_tcp_handler (int sockfd)
{
  TCP_HANDLER handler = malloc(sizeof(struct TCP_SOCKET_HANDLER));
  if (handler == NULL)
    {
      perror ("Unable to allocate memory for TCP_HANDLER");
      return NULL;
    }
  handler->sockfd = sockfd;
  return handler;
}

int
destroy_tcp_handler (TCP_HANDLER handler)
{
  if (handler)
    {
      close (handler->sockfd);
      free (handler);
    }
  return 0;
}

TCP_SERVER
tcp_new_server (int port)
{
  TCP_SERVER server = malloc (sizeof (struct TCP_SERVER_HANDLER));
  if (server == NULL)
    {
      perror ("Error allocating memory for server");
      return NULL;
    }
  server->port = port;
  return server;
}

int
tcp_server_start (TCP_SERVER server)
{
  int sock, optval = 1;
  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror ("Failed to create TCP socket for server");
      return 1;
    }

  TCP_HANDLER handler = new_tcp_handler (sock);
  if (handler == NULL)
    {
      perror ("Error creating TCP_HANDLER in tcp_server_start");
      return 1;
    }
  server->handler = handler;
  if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval,
                  sizeof (optval)) < 0)
    {
      perror ("Could not resuse address");
      return 1;
    }
  struct sockaddr_in sin;
  memset (&sin, 0, sizeof (sin));
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons (server->port);
  if (bind (sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror ("cannot bind socket to address");
      return 1;
    }
  if (listen (sock, MAX_LISTENERS) < 0)
    {
      perror ("error listening");
      return 1;
    }
  return 0;
}

TCP_HANDLER
tcp_server_next_connection(TCP_SERVER server)
{
  struct sockaddr_in addr;
  int client_sock = sizeof (addr);
  socklen_t addr_len = client_sock;
  client_sock = accept (server->handler->sockfd, (struct sockaddr *) &addr, &addr_len);
  if (client_sock < 0)
    {
      perror ("error accepting connection");
      return NULL;
    }
  return new_tcp_handler (client_sock);
}

int
destroy_tcp_sever (TCP_SERVER server)
{
  if (server)
    {
      destroy_tcp_handler (server->handler);
      free (server);
    }
   return 0;
}

TCP_CLIENT_CONN
tcp_new_client (char *host_ip, unsigned short port)
{
  TCP_CLIENT_CONN client = malloc (sizeof (struct TCP_CLIENT_HANDLER));
  if (client == NULL)
    {
      perror ("Unable to allocate memory for TCP_CLIENT_CONN");
      return NULL;
    }
  TCP_HANDLER handler = new_tcp_handler (0);
  client->server_port = port;
  client->host_ip = host_ip;
  client->handler = handler;
  return client;
}

int
tcp_client_connect (TCP_CLIENT_CONN client)
{
  int sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror ("Error creating tcp client connection socket");
      return 1;
    }
  struct sockaddr_in sin;
  unsigned short server_port = client->server_port;
  memset (&sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr (client->host_ip);
  sin.sin_port = htons (server_port);

  if (connect (sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror ("cannot connect to server");
      return 1;
    }
  client->handler->sockfd = sock;
  return 0;
}

int
destroy_tcp_client (TCP_CLIENT_CONN client)
{
  if (client)
    {
      if (client->handler)
        destroy_tcp_handler (client->handler);
      free (client);
    }
  return 0;
}

int
tcp_recvn (TCP_HANDLER handler, char *buf, int buf_len)
{
  int total = 0;
  int remaining = buf_len;
  while (total < buf_len) 
    {
      int received = recv (handler->sockfd, buf + total, remaining, 0);
      if (received < 0)
        {
          perror ("tcp_recvn failed to recv enough data from socket");
          return 1;
        }
      total += received;
      remaining -= received;
    }
  return total;
}

int
tcp_sendn (TCP_HANDLER handler, char *buf, int buf_len)
{
  int total = 0;
  int remaining = buf_len;
  while (total < buf_len)
    {
      int sent = send (handler->sockfd, buf + total, remaining, 0);
      if (sent < 0)
        {
          printf("Error sending data from tcp_sendn");
          return sent;
        }
      total += sent;
      remaining -= sent;
    }
  return 0;
}

int
tcp_send (TCP_HANDLER handler, char *buf, int buf_len)
{
  char num_buf[4];
  uint32_t len_nb = htonl (buf_len);
  num_buf[3] = (len_nb >> 0) & 0xFF;
  num_buf[2] = (len_nb >> 8) & 0xFF;
  num_buf[1] = (len_nb >> 16) & 0xFF;
  num_buf[0] = (len_nb >> 24) & 0xFF;
  int sent_size = tcp_sendn (handler, num_buf, 4);
  if (sent_size)
    {
      perror ("Error with tcp_send failed to sent size");
      return 1;
    }
  int sent_data = tcp_sendn (handler, buf, buf_len);
  if (sent_data)
    {
      perror ("Error sending data with tcp_send");
      return 1;
    }
  return 0;
}

int
tcp_recv (TCP_HANDLER handler, char *buf, int *output_len)
{
  *output_len = 0;
  char num_buf[4];
  int recv_len = tcp_recvn (handler, num_buf, 4);
  if (recv_len < 0)
    {
      perror ("tcp_recvn failed to recv enough data from socket when getting length");
      return 1;
    }
  else if (recv_len == 0)
    {
      printf ("Received EOF from tcp_recvn\n");
      return EOF;
    }
  uint32_t len_nb = (num_buf[0] << 24)
                    | (num_buf[1] << 16)
                    | (num_buf[2] << 8)
                    | (num_buf[3] << 0);
  int size = ntohl (len_nb);
  int received = tcp_recvn (handler, buf, size);
  if (received < 0)
    {
      perror("tcp_recv failed to recv enough data "
             "from socket when getting data");
      return 1;
    }
  else if (recv_len == 0)
    return EOF;
  *output_len = size;
  return 0;
}
