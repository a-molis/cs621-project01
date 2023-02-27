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
  printf("started %d\n", started);
  if (started)
    {
      perror ("Unable to start server");
      abort ();
    }
  TCP_HANDLER client_handler = tcp_server_next_connection (server);
  printf("Sending data on server\n");
  int sent = tcp_send (client_handler, server_args->input, 6);
  printf("Server sent data to client on server_port %d\n", server_port);
  if (sent)
    printf("server failed to send hello from server\n");
  else
    printf("sent Hello from server\n");

  if (destroy_tcp_handler (client_handler))
    printf("Failed to destroy client socket handler in server\n");
  if (destroy_tcp_sever (server))
    printf ("Failed to destroy server handler");

  return 0;
}

void *run_client(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  printf ("Running test with server %s on server_port %s in thread\n", server_args->ip_addr, server_args->port);

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
    printf("Client failed to receive message from server from server %s\n", test);
  else
    printf("Client received message from server: %s\n", test);
  printf("Test udp_server_next_conn %s with output_len of %d\n", test, output_len);
  if (destroy_tcp_client (client))
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
  printf ("Copying data input: %s output: %s\n", server_args->input, server_args->output);
  free(server_args);
  printf("Success = %d\n", success);
  return success;
}