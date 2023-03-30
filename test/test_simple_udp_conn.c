/**
 * Integration test for testing simple UDP client server.
 */
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "udp_sock_handler.h"

// Struct for arguments passed to test threads.
struct args
  {
    char *ip_addr;
    char *port;
    char *input;
    char output[1024];
    int started_client;
    int started_server;
    int input_len;
};

/**
 * Function to be run in a thread for testing a simple UDP server.
 * @param inputs The input pointer, should be a struct args.
 * @return Pointer to return variable.
 */
void
*run_server(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  unsigned short server_port = atoi (server_args->port);

  UDP_SERVER server = udp_new_server (server_port);
  server_args->started_server = udp_server_start (server);
  if (server_args->started_server)
    return 0;
  UDP_HANDLER client_handler = udp_server_next_connection (server);
  if (client_handler == NULL)
    {
      printf("Server failed to get next udp conn");
    }
  char test[server_args->input_len];
  int success = udp_recvfrom_n (client_handler, test, server_args->input_len);
  test[server_args->input_len] = '\0';
  if (success)
    printf("Sever failed to receive test data from client\n");
  strcpy (server_args->output, test);
  if (udp_server_destroy(server))
    printf ("Failed to destroy server client_handler");
  if (udp_destroy_handler (client_handler)) {
    printf("Failed to destroy client client_handler");
  }
  return 0;
}

/**
 * Function to be run in a thread for testing a simple UDP client.
 * @param inputs The input pointer, should be a struct args.
 * @return Pointer to return variable.
 */
void
*run_client(void *inputs)
{
  struct args *server_args = (struct args*) inputs;
  unsigned short port = atoi (server_args->port);
  UDP_CLIENT_CONN client = udp_new_client (server_args->ip_addr, port);
  server_args->started_client = udp_client_connect (client);
  int sent_success = udp_sendto_n (client->handler,
                                 server_args->input, server_args->input_len);
  if (sent_success)
    printf("Failed to send test data from server to client\n");
  if (udp_destroy_client (client))
    printf ("Error destroying client\n");
  return 0;
}

/**
 * Main function for running test for simple UDP server/client.
 * @return Returns 0 if no errors, 1 otherwise.
 */
int
main() {
  pthread_t client_thread, server_thread;
  struct args *server_args = malloc (sizeof (struct args));
  server_args->port =  "2000";
  server_args->ip_addr =  "127.0.0.1";
  server_args->input =  "hello";
  server_args->input_len = 6;
  server_args->started_client = 1;
  server_args->started_client = 1;

  pthread_create (&server_thread, NULL, (void *) &run_server, (void *) server_args);
  sleep (1);
  pthread_create (&client_thread, NULL, (void *) &run_client, (void *) server_args);
  pthread_join (client_thread, NULL);
  pthread_join (server_thread, NULL);
  int success = 0;
  if (server_args->started_client || server_args->started_server
      || (strcmp (server_args->input, server_args->output) != 0))
    success = 1;
  free (server_args);
  return success;
}