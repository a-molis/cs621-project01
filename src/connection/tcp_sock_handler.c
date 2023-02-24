#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
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
    {
      close (handler->sockfd);
      free (handler);
    }
  return 0;
}

TCP_SERVER tcp_new_server(int port)
{
  TCP_SERVER server = malloc (sizeof (struct TCP_SOCKET_HANDLER));
  server->port = port;
  return server;
}

int tcp_server_start(TCP_SERVER server)
{
  int sock, optval = 1;
  if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror ("couldn’t create TCP socket");
      abort ();
    }
  TCP_HANDLER handler = new_tcp_handler (sock);
  server->handler = handler;
  if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval,
                  sizeof (optval)) < 0)
    {
      perror ("Could not resuse address");
      abort ();
    }
  printf("Set up server socket\n");
  struct sockaddr_in sin;
  memset (&sin, 0, sizeof (sin));
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons (server->port);
  printf("Binding server to port %d\n", server->port);
  if (bind (sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror ("cannot bind socket to address");
      return 1;
    }

  printf("Bound server to port %d\n", server->port);
  if (listen (sock, 10) < 0)
    {
      perror ("error listening");
      return 1;
    }
  return 0;
}

TCP_HANDLER tcp_server_next_connection(TCP_SERVER server)
{
  struct sockaddr_in addr;
  int client_sock = sizeof (addr);
  socklen_t addr_len = client_sock;
  printf("Server waiting for incoming connection on port %d\n", server->port);
  client_sock = accept (server->handler->sockfd, (struct sockaddr *)&addr, &addr_len);
  printf("Server made new connection\n");
  if (client_sock < 0)
    {
      perror ("error accepting connection");
      abort ();
    }
  printf("Server made  connection to client on port %d\n", server->port);
  return new_tcp_handler (client_sock);
}

int destroy_tcp_sever(TCP_SERVER server)
{
  if (server)
    {
      destroy_tcp_handler (server->handler);
      free(server);
    }
   return 0;
}

TCP_CLIENT_CONN tcp_new_client(char *host, unsigned short port)
{
  TCP_CLIENT_CONN client = malloc (sizeof (struct TCP_CLIENT_HANDLER));
  client->port = port;
  client->host = host;
  return client;
}

int tcp_client_connect(TCP_CLIENT_CONN client)
{
  int sock = socket (PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in sin;
  struct hostent *host = gethostbyname (client->host);
  in_addr_t server_addr = *(in_addr_t *) host->h_addr_list[0];
  unsigned short server_port = client->port;
  memset (&sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = server_addr;
  sin.sin_port = htons (server_port);

  printf("Client connecting to server on host %s on port %d\n", client->host, server_port);
  if (connect (sock, (struct sockaddr *) &sin, sizeof (sin))<0)
    {
      perror("cannot connect to server");
      return 1;
    }

  printf("Client connected to server on host %s on port %d\n", client->host, server_port);
  TCP_HANDLER handler = new_tcp_handler (sock);
  client->handler = handler;
  printf("Created new tcp handler in client\n");
  return 0;
}

int destroy_tcp_client(TCP_CLIENT_CONN client)
{
  if (client)
    {
      destroy_tcp_handler (client->handler);
      free(client);
    }
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
      perror ("Error with tcp_send failed to sent size");
      return 1;
    }
  int sent_data = tcp_sendn(handler, buf, buf_len);
  if (sent_data == 0)
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
                    | (num_buf[1] << 16)
                    | (num_buf[2] << 8)
                    | (num_buf[3] << 0);
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



