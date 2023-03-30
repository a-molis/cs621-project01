/**
 * Integration test for testing simple TCP client server.
 */
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "tcp_sock_handler.h"

// Struct for arguments passed to test threads.
struct args
{
    char *ip_addr;
    char *port;
    char *input;
    char output[1024];
};

/**
 * Function to be run in a thread for testing a simple TCP server.
 * @param inputs The input pointer, should be a struct args.
 * @return Pointer to return variable.
 */
void
*run_server(void *inputs)
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
  int sent = tcp_sendn(client_handler, server_args->input, 6);
  if (sent)
    printf("server failed to send hello from server\n");

  if (destroy_tcp_handler (client_handler))
    printf("Failed to destroy client socket handler in server\n");
  if (destroy_tcp_sever (server))
    printf ("Failed to destroy server handler");

  return 0;
}

/**
 * Function to be run in a thread for testing a simple TCP client.
 * @param inputs The input pointer, should be a struct args.
 * @return Pointer to return variable.
 */
void
*run_client(void *inputs)
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
  char test[6] = {'\0'};
  int received = tcp_recvn (client->handler, test, 6);
  if (!received)
    printf("Client failed to receive message from server from server %s\n", test);
  if (destroy_tcp_client (client))
    printf("Failed to destroy client socket handler\n");
  strcpy (server_args->output, test);
  return 0;
}

/**
 * Main function for running test for simple TCP server/client.
 * @return Returns 0 if no errors, 1 otherwise.
 */
int
main() {
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
