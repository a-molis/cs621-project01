/**
 * System test for part 2.
 */
#include <stdio.h>
#include "comp_utils.h"

/**
 * Runs part 2 as a test.
 * @return Returns 0 if no errors, 1 otherwise.
 */
int
main ()
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