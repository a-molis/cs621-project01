#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "udp_sock_handler.h"
#include "tcp_sock_handler.h"
#include "config.h"

int open_file (char *path, char *buf);
int client_pre_probe (CONFIG config, char *config_str);

int main(int argc, char *argv[])
{
  char *config_path = argv[1];
  if (!config_path)
    {
      printf ("Missing required arg config\n");
      exit (1);
    }
  char buf[MAX_TCP_SIZE];
  int opened = open_file(config_path, buf);
  if (opened)
    {
      printf ("Failed to open config file\n");
      exit (1);
    }
  // TODO send config to server
  CONFIG config = config_new(buf);
  printf ("config server ip %s\n", config->server_ip);

  int pre_probe = client_pre_probe (config, buf);
  config_destroy(config);
  return 0;
}

int client_pre_probe (CONFIG config, char *config_str)
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

int open_file (char *path, char *buf)
{
  FILE *fd = fopen(path, "r");
  if (!fd)
    {
      printf("Failed to open file %s\n", path);
      return 1;
    }
  int size = 1024;
  size_t offset = 0;
  while (fgets(buf + offset, size, fd) != NULL)
    {
      offset = strlen(buf);
    }
  if (fclose (fd))
    {
      perror ("Unable to close file");
      abort ();
    }
  printf("\n %s\n", buf);
  return 0;
}

