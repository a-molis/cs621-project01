#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "udp_sock_handler.h"

int udp_setup_client (UDP_CLIENT_CONN client);

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
  server->sockfd = 0;
  return server;
}

int udp_server_destroy(UDP_SERVER server)
{
  if (server)
    {
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
  struct sockaddr_in sin;
  memset (&sin, 0, sizeof (sin));
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons (server->port);
  sin.sin_family = AF_INET;

  server->addr = &sin;
  server->addr_len =  sizeof (sin);

  printf("Binding server to server_port %d\n", server->port);
  if (bind (sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror ("cannot bind socket to address");
      return 1;
    }
  printf("Bound server to server_port %d\n", server->port);

  return 0;
}

UDP_HANDLER udp_server_next_connection (UDP_SERVER server)
{
  struct sockaddr_in sout;
  UDP_HANDLER handler = udp_new_handler (server->sockfd);
  handler->addr = &sout;
  handler->addr_len = sizeof (sout);
  char start[MAX_UDP_SIZE];
  int output_len = 0;

  ssize_t received = udp_recvfrom (handler, start, &output_len);
  printf("Server received initial message with %lu bytez %s \n", received, start);
  char *test_message = "confirm";
  ssize_t sent = udp_sendto (handler, test_message, 8);
  if (sent < 0)
    {
      perror ("Unable to send message");
      abort ();
    }
  printf("Server sent %zu bytes to client\n", sent);
  return handler;
}

UDP_CLIENT_CONN udp_new_client(char *ip_address, unsigned short port)
{
  UDP_CLIENT_CONN client = malloc (sizeof (struct UDP_CLIENT_HANDLER));
  UDP_HANDLER handler = udp_new_handler (0);
  handler->addr = NULL;
  client->port = port;
  client->ip_address = ip_address;
  client->handler = handler;
  return client;
}

// TODO pull this logic out to common function between TCP and UDP
int udp_client_connect(UDP_CLIENT_CONN client)
{
  // TODO check if new error handling needed
  int sock;
  if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("couldn’t create TCP socket");
      abort ();
    }
  printf ("set up client socket\n");

  struct sockaddr_in sin;
  memset (&sin, 0, sizeof (sin));
  sin.sin_addr.s_addr = inet_addr(client->ip_address);
  sin.sin_port = htons (client->port);
  sin.sin_family = AF_INET;

  client->handler->addr = &sin;
  client->handler->addr_len = sizeof (sin);


  client->handler->sockfd = sock;
  char *test_message = "start";
  int setup = udp_setup_client (client);
  printf("setup %d\n", setup);
  if (setup)
    return 1;
  printf("Created new upd handler in client\n");
  return 0;
}

int udp_setup_client (UDP_CLIENT_CONN client)
{
  char *mess = "start";
  int sent = udp_sendto (client->handler, mess, 5);
  if (sent)
    return 1;
  char confirm[MAX_UDP_SIZE];
  int output_len = 0;
  int received = udp_recvfrom (client->handler, confirm, &output_len);
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
  printf("Sent %d bytes of data\n", total);
  return 0;
}

int udp_recvfrom_n(UDP_HANDLER handler, char *buf, int buf_len)
{
  printf("Trying to receive data with len %d from udp_recvfrom_n\n", buf_len);
  int total = 0;
  int remaining = buf_len;
  while (total < buf_len)
    {
      printf("udp_recvfrom_n total %d\n", total);
      int received = recvfrom (handler->sockfd, buf + total, remaining, 0,
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
      perror("Error sending data with tcp_send");
      return 1;
    }
  return 0;
}

int udp_recvfrom(UDP_HANDLER handler, char *buf, int *output_len)
{
  *output_len = 0;
  char num_buf[4];
  int recv_len = udp_recvfrom_n (handler, num_buf, 4);
  if (recv_len < 0)
    {
      perror ("tcp_recvn failed to recv enough data "
             "from socket when getting length");
      return 1;
    }
  else if (recv_len == 0)
    return EOF;
  uint32_t len_nb = (num_buf[0] << 24)
                    | (num_buf[1] << 16)
                    | (num_buf[2] << 8)
                    | (num_buf[3] << 0);
  int size = ntohl (len_nb);

  *buf = malloc (sizeof(char) * size);
  int received = udp_recvfrom_n (handler, buf, size);
  if (received < 0)
    {
      perror ("tcp_recv failed to recv enough data "
             "from socket when getting data");
      return 1;
    }
  *output_len = size;
  return 0;
}