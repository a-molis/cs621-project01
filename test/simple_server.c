#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "tcp_sock_handler.h"

int main(int argc,char *argv[]) {
  unsigned short server_port = atoi (argv[1]);
  
  int sock = socket (PF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sin;
  memset (&sin, 0, sizeof (sin));
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons (server_port);
  if (bind (sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror ("cannot bind socket to address");
      abort ();
    }
    if (listen (sock, 5) < 0) { 
      perror ("error listening"); 
      abort ();
    }
  struct sockaddr_in addr;
  int client_sock = sizeof (addr);
  socklen_t addr_len = client_sock;
  client_sock = accept (sock, (struct sockaddr *)&addr, &addr_len); 
  if (client_sock < 0)
    {
      perror ("error accepting connection");
      abort (); 
    }

  TCP_HANDLER handler = new_tcp_handler(sock);
    
}
