#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "udp_sock_handler.h"
#include "tcp_sock_handler.h"
#include "config.h"
#include "comp_utils.h"

void comp_client_run (char *config_path);

int main(int argc, char *argv[])
{
  char *config_path = argv[1];
  if (!config_path)
    {
      printf ("Missing required arg config\n");
      exit (1);
    }
  comp_client_run (config_path);
  return 0;
}

void comp_client_run (char *config_path)
{
  char buf[MAX_TCP_SIZE];
  CONFIG config = get_config (config_path, buf);
  printf ("config server ip %s\n", config->server_ip);

  int pre_probe = client_pre_probe (config, buf);
  if (pre_probe)
    {
      perror ("Client failed to pre probe server");
      abort ();
    }
  int probe = client_probe(config);
  if (probe)
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
}
