#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "udp_sock_handler.h"

int main(int argc, char *argv[]) {
  char *host = argv[1];
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
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin.sin_port = htons (port);
    sin.sin_family = AF_INET;
    if (connect (sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
      {
        perror ("failed to connect to server from client\n");
        abort ();
      }
    printf("connected to server from client\n");
    char *test_message = "test hello hello";
    int sent = sendto (sock, test_message, 1000, 0, (struct sockaddr *) NULL, sizeof (sin));
    if (sent < 0)
      {
        perror ("Unable to send message");
        abort ();
      }
    printf("Sent %d bytes of data\n", sent);
//  printf ("Running test with server %s on port %s in thread\n", host, argv[2]);
//
//  UDP_CLIENT_CONN client = udp_new_client (host, port);
//  int connected = udp_client_connect (client);
//  if (connected)
//    {
//      printf ("Unable to connect to server %s on port %d\n", host, port);
//      abort ();
//    }
//  printf("Client sending initial message to server\n");
//  char start[6] = "start\0";
//  int sent = udp_sendto_n (client->handler, (char *) &start, 6);
//  if (sent < 0)
//    {
//      printf("Client sent initial message with %d bytes\n", sent);
//      exit(1);
//    }
//  printf("Client sent initial message with %d bytes\n", sent);
//  char test[6] = {'\0'};
//  int received = udp_recvfrom_n (client->handler, test, 6);
//  printf("Client received initial message with %d bytes\n", received);
//  if (!received)
//    printf("Client failed to receive message from server from server %s\n", test);
//  else
//    printf("Client received message from server: %s\n", test);
//  if (udp_destroy_client (client))
//    printf("Failed to destroy client socket handler\n");
  return 0;
}
