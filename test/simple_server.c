#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "tcp_sock_handler.h"

int main(int argc,char *argv[]) {
  unsigned short server_port = atoi (argv[1]);
 
  int sock, optval = 1;

  if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror ("couldn’t create TCP socket");
      abort (); 
    }
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
  sin.sin_port = htons (server_port);
  printf("Binding server to port %d\n", server_port);
  if (bind (sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror ("cannot bind socket to address");
      abort ();
    }

  printf("Bound server to port %d\n", server_port);
  if (listen (sock, 10) < 0) 
    { 
      perror ("error listening"); 
      abort ();
    }
  struct sockaddr_in addr;
  int client_sock = sizeof (addr);
  socklen_t addr_len = client_sock;

  printf("Server waiting for incoming connection on port %d\n", server_port);
  client_sock = accept (sock, (struct sockaddr *)&addr, &addr_len); 
  printf("Server made new connection\n");
  if (client_sock < 0)
    {
      perror ("error accepting connection");
      abort (); 
    }
  printf("Server made  connection to client on port %d\n", server_port);
  TCP_HANDLER handler = new_tcp_handler (client_sock);
  printf("Sending data on server\n");
  char test[6] = "Hello\0";
  int sent = tcp_sendn(handler, test, 6);
  printf("Server sent data to client on port %d\n", server_port);
  if (sent) 
    printf("server failed to send hello from server\n");
  else
    printf("sent Hello from server\n");

  if (destroy_tcp_handler (handler))
    printf("Failed to destroy client socket handler\n");
  if (close (sock))
    printf("Failed to close client sockfd\n");
  return 0; 
}
