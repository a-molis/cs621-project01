#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "constants.h"
#include "udp_sock_handler.h"

int udp_setup_client (UDP_CLIENT_CONN client);

// TODO add free/destroy
UDP_HANDLER udp_new_handler(int sockfd)
{
  UDP_HANDLER handler = malloc (sizeof (struct UDP_SOCKET_HANDLER));
  if (handler == NULL)
    {
      perror ("Unable to allocate memory for new UDP handler");
      return NULL;
    }
  printf("sock_fd at  new %d\n", sockfd);
  handler->sockfd = sockfd;
  printf("sock_fd at assignment %d\n", handler->sockfd);
  return handler;
}

int udp_destroy_handler(UDP_HANDLER handler)
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

UDP_SERVER udp_new_server(int port)
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

int udp_server_destroy(UDP_SERVER server)
{
  if (server)
    {
      if (server->addr)
        free(server->addr);
      free(server);
    }
  return 0;
}

// TODO decouple start and initial client handler
int udp_server_start(UDP_SERVER server)
{
  int sock, optval = 1;
  if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("couldn’t create TCP socket");
      abort ();
    }
  server->sockfd = sock;
  if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval,
                  sizeof (optval)) < 0)
    {
      perror ("Could not resuse address");
      abort ();
    }
  printf("Set up server socket\n");
  struct sockaddr_in *sin = malloc (sizeof (struct sockaddr_in));
  memset (sin, 0, sizeof (*sin));
  sin->sin_addr.s_addr = INADDR_ANY;
  sin->sin_port = htons (server->port);
  sin->sin_family = AF_INET;

  server->addr = sin;
  server->addr_len =  sizeof (*sin);

  printf("Binding server to server_port %d\n", server->port);
  if (bind (sock, (struct sockaddr *) sin, sizeof (*sin)) < 0)
    {
      perror ("cannot bind socket to address");
      return 1;
    }
  printf("Bound server to server_port %d\n", server->port);

  return 0;
}

UDP_HANDLER udp_server_next_connection (UDP_SERVER server)
{
  struct sockaddr_in *sout = malloc (sizeof (struct sockaddr_in));
  UDP_HANDLER handler = udp_new_handler (server->sockfd);
  if (handler == NULL)
    {
      perror ("Failed to get next UDP connection for server");
      return NULL;
    }
  sout->sin_family = AF_INET;
  handler->addr = sout;
  handler->addr_len = sizeof (*sout);
  return handler;
}

UDP_CLIENT_CONN udp_new_client(char *ip_address, unsigned short port)
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

int udp_client_connect(UDP_CLIENT_CONN client)
{
  int sock;
  if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("Couldn’t create UDP socket");
      return 1;
    }
  printf ("set up client socket\n");
  // TODO verify DF bit set correctly with tcpdump
  int optval = IP_PMTUDISC_DO;
  if (setsockopt (sock, IPPROTO_IP, IP_MTU_DISCOVER, &optval,
                  sizeof (optval)) < 0)
    {
      perror ("Failed to set socket option for DF bit");
      return 1;
    }

  struct sockaddr_in *sin = malloc (sizeof (struct sockaddr_in));
  if (sin == NULL)
    {
      perror ("Error setting up client sockaddr_in");
      return 1;
    }
  memset (sin, 0, sizeof (*sin));
  // TODO update to use inet_pton or check if -1
  sin->sin_addr.s_addr = inet_addr(client->ip_address);
  sin->sin_port = htons (client->port);
  sin->sin_family = AF_INET;
  UDP_HANDLER handler = udp_new_handler (sock);
  client->handler = handler;
  client->handler->addr = sin;
  client->handler->addr_len = sizeof (*sin);

  client->handler->sockfd = sock;
  return 0;
}

int udp_setup_client (UDP_CLIENT_CONN client)
{
  char *mess = "start";
  int sent = udp_sendto_n (client->handler, mess, 5);
  if (sent)
    return 1;
  char confirm[MAX_UDP_SIZE];
  int received = udp_recvfrom_n (client->handler, confirm, 8);
  if (received)
    return 1;
  printf("Client received %d bytes from the server with message %s\n", received, confirm);
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
  int sent = sendto(handler->sockfd, buf, buf_len, 0,
                    (struct sockaddr *)handler->addr, handler->addr_len);
  if (sent < buf_len)
    {
      perror ("Error sending udp datagram");
      return sent;
    }
  return 0;
}

int udp_recvfrom_n(UDP_HANDLER handler, char *buf, int buf_len)
{
  printf("Trying to receive data with len %d from udp_recvfrom_n\n", buf_len);

      int received = recvfrom (handler->sockfd, buf, buf_len, 0,
                              (struct sockaddr *) handler->addr, &handler->addr_len);
      printf ("Received %d bytes\n", received);
      if (received < buf_len)
        {
          perror("udp_recvfrom_n failed to recv enough data from socket");
          return errno;
        }
        return 0;
}

int udp_sendto(UDP_HANDLER handler, char *buf, int buf_len)
{
  printf("Sending data with udp_sendto\n");
  char num_buf[4];
  uint32_t len_nb = htonl(buf_len);
  num_buf[3] = (len_nb >> 0) & 0xFF;
  num_buf[2] = (len_nb >> 8) & 0xFF;
  num_buf[1] = (len_nb >> 16) & 0xFF;
  num_buf[0] = (len_nb >> 24) & 0xFF;
  int sent_size = udp_sendto_n(handler, num_buf, 4);
  if (sent_size)
    {
      perror ("Error with udp_sendto_n failed to sent size");
      return 1;
    }
  int sent_data = udp_sendto_n(handler, buf, buf_len);
  if (sent_data)
    {
      perror("Error sending data with udp_sendto");
      return 1;
    }
  return 0;
}

int udp_recvfrom(UDP_HANDLER handler, char *buf, int *output_len)
{
  *output_len = 0;
  char num_buf[4];
  int recv_len = udp_recvfrom_n (handler, num_buf, 4);
  if (recv_len)
    {
      perror ("tcp_recvn failed to recv enough data "
             "from socket when getting length");
      return 1;
    }
  uint32_t len_nb = (num_buf[0] << 24)
                    | (num_buf[1] << 16)
                    | (num_buf[2] << 8)
                    | (num_buf[3] << 0);
  int size = ntohl (len_nb);

  *buf = malloc (sizeof(char) * size);
  int received = udp_recvfrom_n (handler, buf, size);
  if (received)
    {
      perror ("tcp_recv failed to recv enough data "
             "from socket when getting data");
      return 1;
    }
  *output_len = size;
  return 0;
}