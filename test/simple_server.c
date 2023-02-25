#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "udp_sock_handler.h"

int main(int argc,char *argv[]) {
  unsigned short server_port = atoi (argv[1]);

  UDP_SERVER server = udp_new_server (server_port);
  int started = udp_server_start (server);
  printf("started %d\n", started);
  if (started)
    {
      perror ("Unable to start server");
      abort ();
    }
  sleep(2);
  char test[6] = "Hello\0";
  int sent = udp_sendto_n(server->handler, (char *) &test, 6);
  printf("Server sent data to client on port %d\n", server_port);
  if (sent)
    printf("server failed to send hello from server\n");
  else
    printf("sent Hello from server\n");
  if (udp_server_destroy(server))
    printf ("Failed to destroy server handler");

  return 0;
}
