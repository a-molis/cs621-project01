#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "udp_sock_handler.h"
#include "tcp_sock_handler.h"
#include "config.h"
#include "compdetect.h"

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



