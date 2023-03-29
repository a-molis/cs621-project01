#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "tcp_sock_handler.h"
#include "constants.h"


struct args {
    char *ip_addr;
    char *port;
    char *input;
    char output[1024];
};

void *run_server(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  unsigned short server_port = atoi (server_args->port);

  TCP_SERVER server = tcp_new_server (server_port);
  int started = tcp_server_start (server);
  if (started)
    {
      perror ("Unable to start server");
      abort ();
    }
  TCP_HANDLER client_handler = tcp_server_next_connection (server);
  int sent = tcp_send (client_handler, server_args->input, 6);

  if (destroy_tcp_handler (client_handler))
    printf("Failed to destroy client socket handler in server\n");
  if (destroy_tcp_sever (server))
    printf ("Failed to destroy server handler");

  return 0;
}

void *run_client(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  unsigned short port = atoi (server_args->port);
  TCP_CLIENT_CONN client = tcp_new_client (server_args->ip_addr, port);
  int connected = tcp_client_connect(client);
  if (connected)
    {
      printf ("Unable to connect to server %s on server_port %d\n", server_args->ip_addr, port);
      abort ();
    }
  char test[MAX_TCP_SIZE];
  int output_len = 0;

  // TODO check if needed
  memset (test, '\0', sizeof (MAX_TCP_SIZE));
  int received = tcp_recv (client->handler, test, &output_len);
  if (received)
    {
      printf ("Error receiving data\n");
    }
  if (destroy_tcp_client (client))
    printf("Failed to destroy client socket handler\n");
  strcpy (server_args->output, test);
  return 0;
}

int main() {
  pthread_t client_thread, server_thread;
  struct args *server_args = malloc (sizeof (struct args));
  server_args->port =  "12055";
  server_args->ip_addr =  "127.0.0.1";
  server_args->input =  "hello";

  pthread_create (&server_thread, NULL, (void *) &run_server, (void *) server_args);
  sleep (1);
  pthread_create (&client_thread, NULL, (void *) &run_client, (void *) server_args);
  pthread_join(client_thread, NULL);
  pthread_join(server_thread, NULL);
  int success = 1;
  if (strcmp(server_args->input, server_args->output) == 0)
    success = 0;
  free(server_args);
  return success;
}