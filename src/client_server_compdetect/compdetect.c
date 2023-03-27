#include <stdio.h>
#include <stdlib.h>
#include "comp_utils.h"
#include "udp_sock_handler.h"

// TODO make sure all errors handled
// TODO error check if not run with sudo
// TODO set TTL for udp packet train
// TODO test with valgrind
// TODO add timeout for standalone
// TODO check rubric and project outline to verify all done
// TODO remove all print statements
// TODO update random numbers for TCP send
// TODO make sure single application does not seg fault if not run without root
// TODO create pcap files

int main(int argc, char *argv[])
{
  char *config_path = argv[1];
  if (!config_path)
    {
      printf ("Missing required arg config\n");
      exit (1);
    }
  if (run_compdetect (config_path))
    {
      perror ("Error with run_compdetect");
      abort ();
    }
  return 0;
}

int run_compdetect (char *config_path)
{
  printf ("config path %s\n", config_path);
  char buf[MAX_TCP_SIZE];
  CONFIG config = get_config (config_path, buf);
  if (config == NULL)
    {
      perror ("Unable to get config");
      return 1;
    }
  if (compdetect_single (config))
    {
      perror ("Error running compdetect_single");
      return 1;
    }
  config_destroy (config);
  return 0;
}

