#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "udp_sock_handler.h"
#include "tcp_sock_handler.h"
#include "config.h"

int server_pre_probe (CONFIG config, char *config_str);

int run_server (char *port);
int main(int argc, char *argv[])
{
  char *server_port = argv[1];
  if (!server_port)
    {
      perror ("Missing required server port");
      abort ();
    }
  int ran = run_server(server_port);
  if (ran)
    {
      perror ("Failed to run server");
      abort ();
    }
  return 0;
}

int run_server (char *port)
{
  int pre_probe = server_pre_probe (port);
  config_destroy(config);
}

int server_pre_probe (CONFIG config, char *config_str)
{
  TCP_CLIENT_CONN client_conn = tcp_new_client (config->server_ip, config->tcp_probing_port);
  int sent = tcp_send (client_conn->handler, config_str, strlen (config_str));
  if (sent)
    {
      perror ("Client failed to send config as string");
      abort ();
    }
  if (destroy_tcp_client (client_conn))
    {
      perror ("Client failed to destroy client socket handler");
      abort ();
    }
  return 0;
}


