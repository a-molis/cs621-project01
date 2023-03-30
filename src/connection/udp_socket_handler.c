/**
 * Functions for UDP socket operations and connections.
 */
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "udp_sock_handler.h"

UDP_HANDLER
udp_new_handler (int sockfd)
{
  UDP_HANDLER handler = malloc (sizeof (struct UDP_SOCKET_HANDLER));
  if (handler == NULL)
    {
      perror ("Unable to allocate memory for new UDP handler");
      return NULL;
    }
  handler->sockfd = sockfd;
  return handler;
}

int
udp_destroy_handler (UDP_HANDLER handler)
{
  if (handler)
    {
      if (handler->addr)
        free (handler->addr);
      close (handler->sockfd);
      free (handler);
    }
  return 0;
}

UDP_SERVER
udp_new_server (int port)
{
  UDP_SERVER server = malloc (sizeof (struct UDP_SOCKET_HANDLER));
  if (server == NULL)
    {
      perror ("Unable to allocate memory for new UDP_SERVER");
      return NULL;
    }
  server->port = port;
  server->sockfd = 0;
  return server;
}

int
udp_server_destroy (UDP_SERVER server)
{
  if (server)
    {
      close (server->sockfd);
      if (server->addr)
        free(server->addr);
      free(server);
    }
  return 0;
}

int
udp_server_start (UDP_SERVER server)
{
  int sock, optval = 1;
  if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("couldn’t create TCP socket");
      return 1;
    }
  server->sockfd = sock;
  if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval,
                  sizeof (optval)) < 0)
    {
      perror ("Could not resuse address");
      return 1;
    }
  struct sockaddr_in *sin = malloc (sizeof (struct sockaddr_in));
  memset (sin, 0, sizeof (*sin));
  sin->sin_addr.s_addr = INADDR_ANY;
  sin->sin_port = htons (server->port);
  sin->sin_family = AF_INET;
  server->addr = sin;
  server->addr_len =  sizeof (*sin);

  if (bind (sock, (struct sockaddr *) sin, sizeof (*sin)) < 0)
    {
      perror ("cannot bind socket to address");
      return 1;
    }
  return 0;
}

UDP_HANDLER
udp_server_next_connection (UDP_SERVER server)
{
  struct sockaddr_in *sout = malloc (sizeof (struct sockaddr_in));
  if (sout == NULL)
    {
      perror ("Error allocating memory for sout in udp_server_next_connection");
      return NULL;
    }
  UDP_HANDLER handler = udp_new_handler (server->sockfd);
  if (handler == NULL)
    {
      free (sout);
      perror ("Failed to get next UDP connection for server");
      return NULL;
    }
  sout->sin_family = AF_INET;
  handler->addr = sout;
  handler->addr_len = sizeof (*sout);
  return handler;
}

UDP_CLIENT_CONN
udp_new_client (char *ip_address, unsigned short port)
{
  UDP_CLIENT_CONN client = malloc (sizeof (struct UDP_CLIENT_HANDLER));
    if (client == NULL)
      {
        perror ("Unable to malloc new udp_client_conn");
        return NULL;
      }
  client->port = port;
  client->ip_address = ip_address;
  return client;
}

int
udp_client_connect (UDP_CLIENT_CONN client)
{
  int sock;
  if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("Couldn’t create UDP socket");
      return 1;
    }
  int optval = IP_PMTUDISC_DO;
  if (setsockopt (sock, IPPROTO_IP, IP_MTU_DISCOVER, &optval,
                  sizeof (optval)) < 0)
    {
      if (close (sock))
        printf ("Failed to close socket");
      perror ("Failed to set socket option for DF bit");
      return 1;
    }
  struct sockaddr_in *sin = malloc (sizeof (struct sockaddr_in));
  if (sin == NULL)
    {
      if (close (sock))
        printf ("Failed to close socket");
      perror ("Error setting up client sockaddr_in");
      return 1;
    }
  memset (sin, 0, sizeof (*sin));
  sin->sin_addr.s_addr = inet_addr (client->ip_address);
  sin->sin_port = htons (client->port);
  sin->sin_family = AF_INET;
  UDP_HANDLER handler = udp_new_handler (sock);
  client->handler = handler;
  client->handler->addr = sin;
  client->handler->addr_len = sizeof (*sin);
  client->handler->sockfd = sock;
  return 0;
}

int
udp_client_connect_bind (UDP_CLIENT_CONN client, int src_port)
{
  if (udp_client_connect (client))
    {
      perror ("Failed to set up socket connection for client conn");
      return 1;
    }
  struct sockaddr_in sin;
  memset (&sin, 0, sizeof (struct sockaddr_in));
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons (src_port);
  sin.sin_family = AF_INET;
  if (bind (client->handler->sockfd, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror ("cannot bind socket to address");
      return 1;
    }
  return 0;
}

int
udp_destroy_client (UDP_CLIENT_CONN client)
{
  if (client)
    {
      udp_destroy_handler (client->handler);
      free (client);
    }
  return 0;
}

int
udp_sendto_n (UDP_HANDLER handler, char *buf, int buf_len)
{
  int sent = sendto (handler->sockfd, buf, buf_len, 0,
                     (struct sockaddr *)handler->addr, handler->addr_len);
  if (sent < buf_len)
    {
      perror ("Error sending udp datagram");
      return sent;
    }
  return 0;
}

int
udp_recvfrom_n (UDP_HANDLER handler, char *buf, int buf_len)
{
  int received = recvfrom (handler->sockfd, buf, buf_len, 0,
                           (struct sockaddr *) handler->addr, &handler->addr_len);
  if (received < buf_len)
    {
      return errno;
    }
  return 0;
}
