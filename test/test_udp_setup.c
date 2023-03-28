#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "udp_sock_handler.h"


struct args {
    char *ip_addr;
    char *port;
    char *input;
    char output[1024];
    int started_client;
    int started_server;
};

void *run_server(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  unsigned short server_port = atoi (server_args->port);

  UDP_SERVER server = udp_new_server (server_port);
  server_args->started_server = udp_server_start (server);
  printf("started server in thread %d\n", server_args->started_server);
  UDP_HANDLER client_handler = udp_server_next_connection (server);
  if (udp_server_destroy(server))
    printf ("Failed to destroy server handler");
  if (udp_destroy_handler (client_handler)){
    printf ("unable to destory client handler");
  }
  return 0;
}

void *run_client(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  printf ("Running test with server %s on server_port %s in thread\n", server_args->ip_addr, server_args->port);

  unsigned short port = atoi (server_args->port);
  UDP_CLIENT_CONN client = udp_new_client (server_args->ip_addr, port);
  server_args->started_client = udp_client_connect (client);
  printf("started client in thread %d\n", server_args->started_client);
  if (udp_destroy_client (client))
    {
      printf ("Error Destroying client\n");
    }
  return 0;
}

int main()
{
  pthread_t client_thread, server_thread;
  struct args *server_args = malloc (sizeof (struct args));
  if (server_args == NULL)
    {
      printf ("Failed to allocate memory for server_args");
      return 1;
    }
  memset (server_args, 0, sizeof (struct args));
  server_args->port =  "12055";
  server_args->ip_addr =  "127.0.0.1";
  server_args->input =  "hello";
  server_args->started_client = 1;
  server_args->started_client = 1;


  pthread_create (&server_thread, NULL, (void *) &run_server, (void *) server_args);
  sleep (1);
  pthread_create (&client_thread, NULL, (void *) &run_client, (void *) server_args);
  pthread_join(client_thread, NULL);
  pthread_join(server_thread, NULL);
  int success = 1;
  printf("started server in outside %d\n", server_args->started_server);
  printf("started client in outside %d\n", server_args->started_client);
  if (!server_args->started_client || !server_args->started_server)
    success = 0;
  printf ("Copying data input: %s output: %s\n", server_args->input, server_args->output);
  free(server_args);
  printf("Success = %d\n", success);
  return success;
}