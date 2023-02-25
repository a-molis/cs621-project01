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
  UDP_CLIENT_CONN client = udp_new_client (ip_address, port);
  int connected = udp_client_connect(client);
  if (connected)
    {
      printf("Client unable to connect to server");
    }
  udp_destroy_client (client);
  return 0;
}
