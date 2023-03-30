/**
 * This contains the main function for running the server side compression detection for part 1.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include "constants.h"
#include "udp_sock_handler.h"
#include "tcp_sock_handler.h"
#include "config.h"
#include "comp_utils.h"

/**
 * Runs the server for compression detection for part 1.
 * @param port The port for the server to run with.
 * @return Returns 0 if there are no errors, 1 otherwise.
 */
int run_server (int port);

/**
 * Main function to run compression detection server for part 1.
 * Run with arg 1 as the port the server will run with.
 *
 * @param argc The number of args.
 * @param argv The pointer to the argv array.
 * @return Returns 0 if there are no errors, 1 otherwise.
 */
int
main(int argc, char *argv[])
{
  char *server_port = argv[1];
  if (!server_port)
    {
      printf ("Missing required server server_port\n");
      return 1;
    }
  int port = atoi (server_port);
  if (run_server(port))
    {
      perror ("Failed to run server");
      return 1;
    }
  return 0;
}

int
run_server (int port)
{
  CONFIG config = server_pre_probe (port);
  if (config == NULL)
    {
      perror ("Server failed in pre probe step");
      return 1;
    }
  char result[config->udp_payload_size];
  bzero (result, config->udp_payload_size);
  if (server_probe (config, result))
    {
      config_destroy(config);
      perror ("Client failed to probe server");
      return 1;
    }
  printf ("Result from server results:\n%s\n", result);
  if (server_post_probe(config, result))
    {
      config_destroy(config);
      perror ("Server failed to send post probe");
      return 1;
    }
  config_destroy(config);
  return 0;
}






