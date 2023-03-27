#include <stdio.h>
#include "comp_utils.h"

int
main()
{
  char *config_path = "test/test_config.json";
  char buf[MAX_TCP_SIZE];
  CONFIG config = get_config (config_path, buf);
  if (config == NULL)
    {
      perror ("Unable to get config");
      return 1;
    }
  if (compdetect_single (config))
    {
      perror ("Error running compdetect_single");
      return 1;
    }
  config_destroy (config);
  return 0;
}