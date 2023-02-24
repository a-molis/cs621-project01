#include <stdio.h>
#include <stdlib.h>
#include "tcp_sock_handler.h"

int main(int argc, char *argv[]) {
  char *host = argv[1];
  unsigned short port = atoi (argv[2]);
  TCP_CLIENT_CONN client = tcp_new_client (host, port);
  int connected = tcp_client_connect(client);
  if (connected)
    {
      printf ("Unable to connect to server %s on port %d\n", host, port);
      abort ();
    }
  char test[6] = {'\0'};
  int received = tcp_recvn (client->handler, test, 6);
  if (!received) 
    printf("Client failed to receive message from server from server %s\n", test);
  else
    printf("Client received message from server: %s\n", test);
  if (destroy_tcp_client (client))
    printf("Failed to destroy client socket handler\n");
  return 0;
}
