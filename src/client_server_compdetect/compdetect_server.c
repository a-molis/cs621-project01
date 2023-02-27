#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "udp_sock_handler.h"
#include "tcp_sock_handler.h"
#include "config.h"

CONFIG server_pre_probe (int port);
int run_server (int port);

int main(int argc, char *argv[])
{
  char *server_port = argv[1];
  if (!server_port)
    {
      perror ("Missing required server server_port");
      abort ();
    }

  // TODO update to use more safe function than atoi
  int port = atoi (server_port);
  int ran = run_server(port);
  if (ran)
    {
      perror ("Failed to run server");
      abort ();
    }
  return 0;
}

int run_server (int port)
{

  int pre_probe = server_pre_probe (port);
  if (pre_probe)
    {
      perror ("Server failed in pre probe step");
      abort ();
    }
//  config_destroy(config);
}

CONFIG server_pre_probe (int port)
{
  TCP_SERVER server = tcp_new_server (port);
  int started = tcp_server_start (server);
  printf("started %d\n", started);
  if (started)
    {
      perror ("Unable to start server in pre probe");
      abort ();
    }
  TCP_HANDLER client_handler = tcp_server_next_connection (server);
  char buf[MAX_TCP_SIZE];
  int buf_len = 0;
  int received = tcp_recv (client_handler, buf, &buf_len);
  if (received)
    {
      perror ("Server failed to get config str from client");
      abort ();
    }
  buf[buf_len] = '\0';
  printf ("server received config\n\n %s \n", buf);

  if (destroy_tcp_handler (client_handler) || destroy_tcp_sever (server))
    {
      perror ("Server failed to close tcp conn in pre probe");
      abort ();
    }
  return NULL;
}


