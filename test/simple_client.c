#include <stdio.h>
#include <stdlib.h>
#include "udp_sock_handler.h"

int main(int argc, char *argv[]) {
  char *host = argv[1];
  unsigned short port = atoi (argv[2]);
  printf ("Running test with server %s on port %s in thread\n", host, argv[2]);

  UDP_CLIENT_CONN client = udp_new_client (host, port);
  int connected = udp_client_connect (client);
  if (connected)
    {
      printf ("Unable to connect to server %s on port %d\n", host, port);
      abort ();
    }
  printf("Client sending initial message to server\n");
  char start[6] = "start\0";
  int sent = udp_sendto_n (client->handler, (char *) &start, 6);
  printf("Client sent initial message with %d bytes\n", sent);
  char test[6] = {'\0'};
  int received = udp_recvfrom_n (client->handler, test, 6);
  printf("Client received initial message with %d bytes\n", received);
  if (!received)
    printf("Client failed to receive message from server from server %s\n", test);
  else
    printf("Client received message from server: %s\n", test);
  if (udp_destroy_client (client))
    printf("Failed to destroy client socket handler\n");
  return 0;
}
