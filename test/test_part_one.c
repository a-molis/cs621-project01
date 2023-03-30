/**
 * System test for part 1.
 */
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <signal.h>
#include "udp_sock_handler.h"
#include "constants.h"
#include "config.h"
#include "comp_utils.h"

// Struct for arguments passed to test threads.
struct args
{
    char *config_path;
    char *port;
    int *client_status;
    int *server_status;
    int tcp_dest_tail_syn_port;
};

/**
 * Function to be run in a thread for testing server for part 1.
 * @param inputs The input pointer, should be a struct args.
 * @return Pointer to return variable.
 */
void
*run_server (void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  unsigned short server_port = atoi (server_args->port);
  CONFIG config = server_pre_probe (server_port);
  if (config == NULL)
    {
      printf ("Server failed in pre probe stage\n");
      *server_args->server_status = 1;
    }

  char result[config->udp_payload_size];
  bzero (result, config->udp_payload_size);
  int probe = server_probe (config, result);
  if (probe)
    {
      printf ("Server failed to probe server\n");
      *server_args->server_status = 1;
    }
  if (server_post_probe (config, result))
    {
      printf ("Server failed to send post probe\n");
      *server_args->server_status = 1;
    }
  config_destroy (config);
  return 0;
}

/**
 * Function to be run in a thread for testing client for part 1.
 * @param inputs The input pointer, should be a struct args.
 * @return Pointer to return variable.
 */
void
*run_client (void *inputs)
{
  struct args *server_args = (struct args*) inputs;

  char buf[MAX_TCP_SIZE];
  CONFIG config = get_config (server_args->config_path, buf);

  int pre_probe = client_pre_probe (config, buf);
  if (pre_probe)
    {
      printf ("Client failed to pre probe server");
      *server_args->client_status = 1;
    }
  int probe = client_probe (config);
  if (probe)
    {
      printf ("Client failed to probe server\n");
      *server_args->client_status = 1;
    }
  // Sleep for enough time for server to open server connection.
  int sleep_time = config->inter_measure_time * 1.7;
  sleep (sleep_time);
  int post_probe = client_post_probe (config);
  if (post_probe)
    {
      printf ("Client error in post probe stage\n");
      *server_args->client_status = 1;
    }
  config_destroy (config);
  return 0;
}

/**
 * Main function for running test for part 1.
 * @return Returns 0 if no errors, 1 otherwise.
 */
int
main() {
  pthread_t client_thread, server_thread;
  struct args *server_args = malloc (sizeof (struct args));
  server_args->port =  "12062";
  server_args->config_path = "test/test_config.json";
  server_args->tcp_dest_tail_syn_port = 12061;
  int server_status = 0;
  int client_status = 0;
  server_args->server_status = &server_status;
  server_args->client_status = &client_status;

  pthread_create (&server_thread, NULL, (void *) &run_server, (void *) server_args);
  sleep (1);
  sigset_t set;
  sigemptyset (&set);
  sigaddset (&set, SIGALRM);
  pthread_sigmask (SIG_BLOCK, &set, NULL);
  pthread_create (&client_thread, NULL, (void *) &run_client, (void *) server_args);
  pthread_join (client_thread, NULL);
  pthread_join (server_thread, NULL);
  int success = 1;
  if (!*server_args->client_status && !*server_args->server_status)
    success = 0;
  free (server_args);
  return success;
}
