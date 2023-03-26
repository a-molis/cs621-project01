#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "udp_sock_handler.h"
#include "tcp_sock_handler.h"
#include "config.h"
#include "comp_utils.h"

int comp_client_run (char *config_path);

int main(int argc, char *argv[])
{
  char *config_path = argv[1];
  if (!config_path)
    {
      printf ("Missing required arg config\n");
      abort ();
    }
  if (comp_client_run (config_path))
    {
      printf ("Error running client\n");
      abort ();
    }
  return 0;
}

int comp_client_run (char *config_path)
{
  char buf[MAX_TCP_SIZE];
  CONFIG config = get_config (config_path, buf);
  if (config == NULL)
    {
      perror ("Error getting config for client");
      return 1;
    }

  int pre_probe = client_pre_probe (config, buf);
  if (pre_probe)
    {
      perror ("Client failed to pre probe server");
      abort ();
    }
  if (client_probe(config))
    {
      perror ("Client failed to probe server");
      abort ();
    }
  // TODO test without sleep
  sleep (5);
  if (client_post_probe (config))
    {
      perror ("Client error in post probe stage");
      abort ();
    }
  config_destroy(config);
  return 0;
}
