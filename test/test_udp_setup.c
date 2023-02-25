#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "udp_sock_handler.h"


struct args {
    char *host;
    char *port;
    char *input;
    char output[1024];
};

void *run_server(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  unsigned short server_port = atoi (server_args->port);

  UDP_SERVER server = udp_new_server (server_port);
  int started = udp_server_start (server);
  printf("started %d\n", started);
  if (started)
    {
      perror ("Unable to start server");
      abort ();
    }
  sleep(2);
  int sent = udp_sendto_n(server->handler, server_args->input, 6);
  printf("Server sent data to client on port %d\n", server_port);
  if (sent)
    printf("server failed to send hello from server\n");
  else
    printf("sent Hello from server\n");
  if (udp_server_destroy(server))
    printf ("Failed to destroy server handler");

  return 0;
}

void *run_client(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  printf ("Running test with server %s on port %s in thread\n", server_args->host, server_args->port);

  unsigned short port = atoi (server_args->port);
  UDP_CLIENT_CONN client = udp_new_client (server_args->host, port);
  int connected = udp_client_connect (client);
  if (connected)
    {
      printf ("Unable to connect to server %s on port %d\n", server_args->host, port);
      abort ();
    }
  char test[6] = {'\0'};
  int received = udp_recvfrom_n (client->handler, test, 6);
  if (!received)
    printf("Client failed to receive message from server from server %s\n", test);
  else
    printf("Client received message from server: %s\n", test);
  if (udp_destroy_client (client))
    printf("Failed to destroy client socket handler\n");
  printf ("Copying data test: %s output: %s\n", test, server_args->output);
  strcpy (server_args->output, test);
  printf ("Copying data test: %s output: %s\n", test, server_args->output);
  return 0;
}

int main() {
  pthread_t client_thread, server_thread;
  struct args *server_args = malloc (sizeof (struct args));
  server_args->port =  "12055";
  server_args->host =  "localhost";
  server_args->input =  "hello";

  pthread_create (&server_thread, NULL, (void *) &run_server, (void *) server_args);
  sleep (1);
  pthread_create (&client_thread, NULL, (void *) &run_client, (void *) server_args);
  pthread_join(client_thread, NULL);
  pthread_join(server_thread, NULL);
  int success = 1;
  if (strcmp(server_args->input, server_args->output) == 0)
    success = 0;
  printf ("Copying data input: %s output: %s\n", server_args->input, server_args->output);
  free(server_args);
  printf("Success = %d\n", success);
  return success;
}