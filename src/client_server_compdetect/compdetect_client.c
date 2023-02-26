#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "constants.h"
#include "udp_sock_handler.h"

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
  int parsed = parse_config(config_path);
  return 0;
}


int parse_config (char *path)
{
  char buf[MAX_TCP_SIZE];
  int opened = open_file(path, buf);
  if (opened)
    {
      perror ("Unable to open config file");
      abort ();
    }
  cJSON *json = cJSON_ParseWithLength(buf, strlen (buf));
  cJSON *server_ip = cJSON_GetObjectItem(json, "server_ip");
  cJSON *foo = cJSON_GetObjectItem(json, "foo");
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

