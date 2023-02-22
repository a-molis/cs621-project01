#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "tcp_sock_handler.h"

int main(int argc, char *argv[]) {
  int sock = socket (PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in sin;
  struct hostent *host = gethostbyname (argv[1]);
  in_addr_t server_addr = *(in_addr_t *) host->h_addr_list[0];
  unsigned short server_port = atoi (argv[2]);
  memset (&sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = server_addr;
  sin.sin_port = htons (server_port);
  if (connect (sock, (struct sockaddr *) &sin, sizeof (sin))<0) 
    {
       perror("cannot connect to server");
       abort();
    }
  return 0;
}
