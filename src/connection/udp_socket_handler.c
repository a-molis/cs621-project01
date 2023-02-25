#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include "udp_sock_handler.h"

// TODO add free/destroy
UDP_HANDLER udp_new_handler(int sockfd)
{
  UDP_HANDLER handler = malloc (sizeof (struct UDP_SOCKET_HANDLER));
  printf("sock_fd at  new %d\n", sockfd);
  handler->sockfd = sockfd;
  printf("sock_fd at assignment %d\n", handler->sockfd);
  return handler;
}

int udp_destroy_handler(UDP_HANDLER handler)
{
  if (handler)
    {
      close (handler->sockfd);
      free (handler);
    }
  return 0;
}

UDP_SERVER udp_new_server(int port)
{
  UDP_SERVER server = malloc (sizeof (struct UDP_SOCKET_HANDLER));
  server->port = port;
  server->handler = NULL;
  return server;
}

int udp_server_destroy(UDP_SERVER server)
{
  if (server)
    {
      udp_destroy_handler(server->handler);
      free(server);
    }
  return 0;
}

int udp_server_start(UDP_SERVER server)
{
  int sock, optval = 1;
  if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("couldn’t create TCP socket");
      abort ();
    }
  server->handler = udp_new_handler(sock);
  server->handler->addr = NULL;
  if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval,
                  sizeof (optval)) < 0)
    {
      perror ("Could not resuse address");
      abort ();
    }
  printf("Set up server socket\n");
  struct sockaddr_in sin;
  memset (&sin, 0, sizeof (sin));
  sin.sin_addr.s_addr = htonl (INADDR_ANY);
  sin.sin_port = htons (server->port);
  sin.sin_family = AF_INET;
  printf("Binding server to port %d\n", server->port);
  if (bind (sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror ("cannot bind socket to address");
      return 1;
    }

  printf("Bound server to port %d\n", server->port);
  return 0;
}

UDP_CLIENT_CONN udp_new_client(char *host, unsigned short port)
{
  UDP_CLIENT_CONN client = malloc (sizeof (struct UDP_CLIENT_HANDLER));
  UDP_HANDLER handler = udp_new_handler (0);
  handler->addr = NULL;
  client->port = port;
  client->host = host;
  client->handler = handler;
  return client;
}

// TODO pull this logic out to common function between TCP and UDP
int udp_client_connect(UDP_CLIENT_CONN client)
{
  // TODO check if new error handling needed
  int sock = socket (AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in sin;
  struct hostent *host = gethostbyname (client->host);
  in_addr_t server_addr = *(in_addr_t *) host->h_addr_list[0];
  unsigned short server_port = client->port;
  memset (&sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = server_addr;
  sin.sin_port = htons (server_port);
  client->handler->addr = &sin;
  client->handler->addr_len = sizeof (sin);

  printf("Client connecting to server on host %s on port %d with sock %d\n", client->host, server_port, sock);
  if (connect (sock, (struct sockaddr *) &sin, sizeof (sin))<0)
    {
      perror("Cannot connect to server");
      return 1;
    }
  char *test_send = "Foo\0";
  int sent = sendto(sock, test_send, 4, 0, (struct sockaddr*)NULL, sizeof(sin));
  if (sent < 0)
    {
      perror ("Error sending initall data from client");
      abort ();
    }
  printf("Sent data %d\n", sent);
  printf("Client connected to server on host %s on port %d\n", client->host, server_port);
  client->handler->sockfd = sock;
  printf("Created new upd handler in client\n");
  return 0;
}

int udp_destroy_client(UDP_CLIENT_CONN client)
{
  if (client)
    {
      udp_destroy_handler(client->handler);
      free(client);
    }
  return 0;
}

int udp_sendto_n(UDP_HANDLER handler, char *buf, int buf_len)
{
  int total = 0;
  int remaining = buf_len;

  while (total < buf_len)
    {
      // TODO see if need to change to ssize_t
      int sent = sendto(handler->sockfd, buf + total, remaining, 0,
                        (struct sockaddr *)handler->addr, handler->addr_len);
      if (sent < 1)
        {
          perror ("Error sending udp datagram");
          return sent;
        }
      total += sent;
      remaining -= sent;
    }
    return 0;
}

int udp_recvfrom_n(UDP_HANDLER handler, char *buf, int buf_len)
{
  printf("Trying to receive data with len %d from udp_recvfrom_n\n", buf_len);
  int total = 0;
  int remaining = buf_len;
  if (handler->addr == NULL)
    {
      printf("addr null creating new addr\n");
      struct sockaddr_in new_addr;
      handler->addr = &new_addr;
      handler->addr_len = sizeof (*handler->addr);
    }
  while (total < buf_len)
    {
      printf("udp_recvfrom_n total %d\n", total);
      int received = recvfrom(handler->sockfd, buf + total, remaining, 0,
                              (struct sockaddr *) handler->addr, &handler->addr_len);
      printf("Number of bytes received %d\n", received);
      if (received < 0)
        {
          perror("udp_recvfrom_n failed to recv enough data from socket");
          return 1;
        }
      total += received;
      remaining -= received;
      printf("Total in udp_recvfrom_n: %d\n", total);
    }
  return total;
}