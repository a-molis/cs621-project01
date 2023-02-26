#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "udp_sock_handler.h"

struct args {
    char *host;
    char *port;
    char *input;
    char output[1024];
    int started_client;
    int started_server;
    int input_len;
};

void *run_server(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  unsigned short server_port = atoi (server_args->port);

  UDP_SERVER server = udp_new_server (server_port);
  server_args->started_server = udp_server_start (server);
  if (server_args->started_server)
    return 0;
  UDP_HANDLER client_handler = udp_server_next_connection (server);
  char test[MAX_UDP_SIZE];
  int output_len = 0;
  int success = udp_recvfrom (client_handler, test, &output_len);
  if (success)
    printf("Failed to send test data from server to client\n");
  strcpy (server_args->output, test);
  if (udp_server_destroy(server))
    printf ("Failed to destroy server client_handler");
  if (udp_destroy_handler (client_handler)) {
    printf("Failed to destroy client client_handler");
  }
  return 0;
}

void *run_client(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  printf ("Running test with server %s on port %s in thread\n", server_args->host, server_args->port);

  unsigned short port = atoi (server_args->port);
  UDP_CLIENT_CONN client = udp_new_client (server_args->host, port);
  server_args->started_client = udp_client_connect (client);
  printf("started client in thread %d\n", server_args->started_client);

  int sent_success = udp_sendto (client->handler,
                                 server_args->input, server_args->input_len);
  if (sent_success)
    printf("Failed to send test data from server to client\n");
  return 0;
}

int main() {
  pthread_t client_thread, server_thread;
  struct args *server_args = malloc (sizeof (struct args));
  server_args->port =  "5000";
  server_args->host =  "127.0.0.1";
  server_args->input =  "hello";
  server_args->input_len = 6;
  server_args->started_client = 1;
  server_args->started_client = 1;

  pthread_create (&server_thread, NULL, (void *) &run_server, (void *) server_args);
  sleep (1);
  pthread_create (&client_thread, NULL, (void *) &run_client, (void *) server_args);
  pthread_join(client_thread, NULL);
  pthread_join(server_thread, NULL);
  int success = 0;
  printf("started server in outside %d\n", server_args->started_server);
  printf("started client in outside %d\n", server_args->started_client);
  if (server_args->started_client || server_args->started_server ||
  (strcmp(server_args->input, server_args->output) != 0))
    success = 1;
  printf ("Copying data input: %s output: %s\n", server_args->input, server_args->output);
  free(server_args);
  printf("Success = %d\n", success);
  return success;
}