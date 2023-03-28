#include <stdio.h>
#include <stdlib.h>
#include "comp_utils.h"
#include "udp_sock_handler.h"

// TODO test with valgrind
// TODO update random numbers for TCP send
// TODO see if only errors with errno need to have perror
// TODO document in README
// TODO update times in the client server code or setup with thread
// TODO create pcap files
// TODO check rubric and project outline to verify all done
// TODO verify if do not fragment bit set correctly
// TODO add comments to all functions
// TODO check if code follows GNU style
// TODO update times
// TODO make sure all errors handled
// TODO remove all printfs
// TODO uncomment all code
// TODO better error handling around unable ot connect to server with different ip address
// TODO all other TODOS
// TODO make sure no magic numbers
// TODO make sure all config variables used
// TODO remove compiler warnings
// TODO clean up makefile
// TODO verify all caps ok for typedef variables
// TODO error check if not run with sudo
// TODO refactor to move raw logic to other file
// TODO make sure single application does not seg fault if not run without root
// TODO move all free error check functions in wrapper functions


int main(int argc, char *argv[])
{
  char *config_path = argv[1];
  if (!config_path)
    {
      printf ("Missing required arg config\n");
      return 1;
    }
  if (run_compdetect (config_path))
    {
      perror ("Error with run_compdetect");
      return 1;
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
      config_destroy (config);
      return 1;
    }
  config_destroy (config);
  return 0;
}

