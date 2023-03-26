#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
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

  printf("servver status in thread %d\n", *server_args->server_status);
  printf("config->tcp_dest_tail_syn_port %d\n", config->tcp_dest_tail_syn_port);
  printf("server_args->tcp_dest_tail_syn_port %d\n", server_args->tcp_dest_tail_syn_port);
  printf("eq %d\n", config->tcp_dest_tail_syn_port == server_args->tcp_dest_tail_syn_port);
  int status = !(config->tcp_dest_tail_syn_port == server_args->tcp_dest_tail_syn_port);
  *server_args->server_status = status;
  printf("server status %d\n", *server_args->server_status);
  printf("status %d\n", status);
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
    printf ("Client failed to pre probe server");
  printf("client pre_probe %d\n", pre_probe);
  *server_args->client_status = pre_probe;
  config_destroy(config);

  return 0;
}

int main() {
  pthread_t client_thread, server_thread;
  struct args *server_args = malloc (sizeof (struct args));
  server_args->port =  "12062";
  
  // TODO update tests to use test_config.json
  server_args->config_path = "myconfig.json";
  server_args->tcp_dest_tail_syn_port = 12061;
  int server_status = 1;
  int client_status = 1;
  server_args->server_status = &server_status;
  server_args->client_status = &client_status;

  pthread_create (&server_thread, NULL, (void *) &run_server, (void *) server_args);
  sleep (1);
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