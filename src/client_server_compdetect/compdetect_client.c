/**
 * Main function for client code for part 1.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "udp_sock_handler.h"
#include "tcp_sock_handler.h"
#include "config.h"
#include "comp_utils.h"

/**
 * Runs the client for compression detection for part 1.
 * @param config_path The path to the configuration json.
 * @return Returns 0 if there are no errors, 1 otherwise.
 */
int
comp_client_run (char *config_path);

/**
 * Main function to run compression detection client for part 1.
 * Run with arg 1 as the path to the config file.
 * @param argc The number of args.
 * @param argv The pointer to the argv array.
 * @return Returns 0 if there are no errors, 1 otherwise.
 */
int
main(int argc, char *argv[])
{
  char *config_path = argv[1];
  if (!config_path)
    {
      printf ("Missing required arg config\n");
      return 1;
    }
  if (comp_client_run (config_path))
    {
      printf ("Error running client\n");
      return 1;
    }
  return 0;
}

int
comp_client_run (char *config_path)
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
  // Sleep for enough time for server to open server connection.
  int sleep_time = config->inter_measure_time * 1.7;
  sleep (sleep_time);
  if (client_post_probe (config))
    {
      perror ("Client error in post probe stage");
      abort ();
    }
  config_destroy(config);
  return 0;
}
