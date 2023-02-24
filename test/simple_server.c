#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "tcp_sock_handler.h"

int main(int argc,char *argv[]) {
  unsigned short server_port = atoi (argv[1]);

  TCP_SERVER server = tcp_new_server (server_port);
  int started = tcp_server_start (server);
  if (!started)
    {
      perror ("Unable to start server");
      abort ();
    }
  TCP_HANDLER client_handler = tcp_server_next_connection (server);
  printf("Sending data on server\n");
  char test[6] = "Hello\0";
  int sent = tcp_sendn(client_handler, test, 6);
  printf("Server sent data to client on port %d\n", server_port);
  if (sent) 
    printf("server failed to send hello from server\n");
  else
    printf("sent Hello from server\n");

  if (destroy_tcp_handler (client_handler))
    printf("Failed to destroy client socket handler in server\n");
  if (destroy_tcp_sever (server))
    printf ("Failed to destroy server handler");
  return 0;
}
