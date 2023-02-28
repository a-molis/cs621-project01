#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcp_sock_handler.h>
#include "compdetect.h"
#include "config.h"
#include "constants.h"



int client_pre_probe (CONFIG config, char *config_str)
{
  TCP_CLIENT_CONN client_conn = tcp_new_client (config->server_ip, config->tcp_probing_port);
  int connected = tcp_client_connect(client_conn);
  if (connected)
    {
      perror ("Client Failed to connect to server in pre probe");
      abort ();
    }
  int sent = tcp_send (client_conn->handler, config_str, strlen (config_str));
  if (sent)
    {
      perror ("Client failed to send config as string");
      return 1;
    }
  if (destroy_tcp_client (client_conn))
    {
      perror ("Client failed to destroy client socket handler");
      return 1;
    }
  return 0;
}

CONFIG server_pre_probe (int port)
{
  TCP_SERVER server = tcp_new_server (port);
  int started = tcp_server_start (server);
  printf("started %d\n", started);
  if (started)
    {
      perror ("Unable to start server in pre probe");
      abort ();
    }
  TCP_HANDLER client_handler = tcp_server_next_connection (server);
  char buf[MAX_TCP_SIZE];
  int buf_len = 0;
  int received = tcp_recv (client_handler, buf, &buf_len);
  if (received)
    {
      perror ("Server failed to get config str from client");
      abort ();
    }
  buf[buf_len] = '\0';
  printf ("server received config\n\n %s \n", buf);

  if (destroy_tcp_handler (client_handler) || destroy_tcp_sever (server))
    {
      perror ("Server failed to close tcp conn in pre probe");
      abort ();
    }
  CONFIG config = config_new (buf);
  return config;
}

int client_probe(CONFIG config)
{

}
