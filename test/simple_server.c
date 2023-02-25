#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include "udp_sock_handler.h"

int main(int argc,char *argv[]) {
  unsigned short server_port = atoi (argv[1]);
  UDP_SERVER server = udp_new_server (server_port);
  int started = udp_server_start (server);
  if (started)
    {
      printf("unable to start UDP server\n");
    }
  udp_server_destroy (server);
  return 0;
}
