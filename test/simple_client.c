#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "udp_sock_handler.h"

int main(int argc, char *argv[]) {
  char *ip_address = argv[1];
  unsigned short port = atoi (argv[2]);
  int sock;
  if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("couldn’t create TCP socket");
      abort ();
    }
  printf ("set up client socket\n");
  struct sockaddr_in sin;
  memset (&sin, 0, sizeof (sin));
  sin.sin_addr.s_addr = inet_addr(ip_address);
  sin.sin_port = htons (port);
  sin.sin_family = AF_INET;
  printf("connected to server from client\n");
  char *test_message = "start";
  int sent = sendto (sock, test_message, 5, 0, (struct sockaddr *) &sin, sizeof (sin));
  if (sent < 0)
    {
      perror ("Unable to send message");
      abort ();
    }

  printf("Sent %d bytes of data\n", sent);
  char confirm[8] = { '\0' };
  socklen_t len = sizeof (sin);
  ssize_t received = recvfrom (sock, confirm, 8, 0, (struct sockaddr *) &sin, &len);
  printf("Client received %zu bytes from the server with message %s\n", received, confirm);
  close(sock);
  return 0;
}
