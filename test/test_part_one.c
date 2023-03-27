#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <signal.h>
#include "udp_sock_handler.h"
#include "constants.h"
#include "config.h"
#include "comp_utils.h"

struct args {
    char *config_path;
    char *port;
    int *client_status;
    int *server_status;
    int tcp_dest_tail_syn_port;
};

// TODO update to test probe not just set up
void *run_server(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  unsigned short server_port = atoi (server_args->port);
  CONFIG config = server_pre_probe (server_port);
  printf ("config tcp_dest_tail_syn_port %d\n", server_args->tcp_dest_tail_syn_port);
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
  printf ("Result from server results:\n%s\n", result);
  if (server_post_probe(config, result))
    {
      printf ("Server failed to send post probe\n");
      *server_args->server_status = 1;
    }
  printf("server status %d\n", *server_args->server_status);
  config_destroy (config);
  return 0;
}

void *run_client(void *inputs)
{
  struct args *server_args = (struct args*) inputs;

  char buf[MAX_TCP_SIZE];
  CONFIG config = get_config (server_args->config_path, buf);
  printf ("config server ip %s\n", config->server_ip);

  int pre_probe = client_pre_probe (config, buf);
  if (pre_probe)
    {
      printf ("Client failed to pre probe server");
      *server_args->client_status = 1;
    }
  int probe = client_probe(config);
  if (probe)
    {
      printf ("Client failed to probe server\n");
      *server_args->client_status = 1;
    }
  // TODO update timing in client server
  // TODO see if we can use thread for server at the end?
  sleep (25);
  int post_probe = client_post_probe (config);
  if (post_probe)
    {
      printf ("Client error in post probe stage\n");
      *server_args->client_status = 1;
    }
  config_destroy(config);
  return 0;
}

int main() {
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
  pthread_join(client_thread, NULL);
  pthread_join(server_thread, NULL);
  int success = 1;
  if (!*server_args->client_status && !*server_args->server_status)
    success = 0;
  free(server_args);
  printf("Success = %d\n", success);
  return success;
}