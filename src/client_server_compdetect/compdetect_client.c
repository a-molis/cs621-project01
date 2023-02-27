#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "udp_sock_handler.h"
#include "config.h"

int parse_config (char *path);
int open_file (char *path, char *buf);

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
  struct CONFIG_DATA * config = config_new(buf);
  printf ("config server ip %s\n", config->server_ip);
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

  printf("\n %s\n", buf);
  return 0;
}

