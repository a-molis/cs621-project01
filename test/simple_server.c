#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "udp_sock_handler.h"

int main(int argc,char *argv[]) {
  unsigned short server_port = atoi (argv[1]);

//  UDP_SERVER server = udp_new_server (server_port);
//  int started = udp_server_start (server);
//  printf("started %d\n", started);
//  if (started)
//    {
//      perror ("Unable to start server");
//      abort ();
//    }
//  printf("Server receiving initial message from client\n");
  int sockfd, optval = 1;
  if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("couldn’t create TCP socket");
      abort ();
    }
  printf("Set up server socket\n");
  struct sockaddr_in sin, sout;
  memset (&sin, 0, sizeof (sin));
  sin.sin_addr.s_addr = htonl (INADDR_ANY);
  sin.sin_port = htons (server_port);
  sin.sin_family = AF_INET;

  printf("Binding server to port %d\n", server_port);
  if (bind (sockfd, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror ("cannot bind socket to address");
      return 1;
    }

  printf("Bound server to port %d\n", server_port);

  char start[4] = {'\0'};
  int struct_sz = sizeof (sout);
  int received = recvfrom (sockfd, start, 4, 0, (struct sockaddr *) &sout, &struct_sz);
//  int received = udp_recvfrom_n (server->handler, start, 4);
  printf("Server received initial message with %d bytes\n", received);
  close(sockfd);
//
//  char test[6] = "Hello\0";
//  int sent = udp_sendto_n(server->handler, (char *) &test, 6);
//  printf("Server sent data to client on port %d\n", server_port);
//  if (sent)
//    printf("server failed to send hello from server\n");
//  else
//    printf("sent Hello from server\n");
//  if (udp_server_destroy(server))
//    printf ("Failed to destroy server handler");

  return 0;
}
