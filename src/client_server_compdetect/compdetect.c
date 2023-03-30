/**
 * Main function for running compression detection for part 2.
 */
#include <stdio.h>
#include <stdlib.h>
#include "comp_utils.h"
#include "udp_sock_handler.h"

// TODO test with valgrind
// TODO update times in the client server code or setup with thread
// TODO create pcap files
// TODO remove random sleeps
// TODO get alarm time from config/constants.h
// TODO check rubric and project outline to verify all done
// TODO check if code follows GNU style
  // TODO space after every function call
// TODO update times
// TODO make sure all errors handled
// TODO uncomment all code
// TODO better error handling around unable ot connect to server with different ip address
// TODO all other TODOS
// TODO make sure no magic numbers
// TODO make sure all config variables used
// TODO remove compiler warnings
// TODO clean up makefile
// TODO verify all caps ok for typedef variables
// TODO error check if not run with sudo
// TODO refactor to move raw logic to other file
// TODO make sure single application does not seg fault if not run without root
// TODO move all free error check functions in wrapper functions
// TODO remove test/simple_client.c and test/simple_server.c
// TODO check if using config->udp_source_port
// TODO add error handling around close function calls
// TODO explain makefile in readme
// TODO check if udp_dest_port and udp_src_port should be different
// TODO check what happens when client runs without server or args
// TODO check what happens if requried arg not present.
// TODO print invalid in part 1 if invalid results
// TODO filter out any ARP messages in pcap files
// TODO check if Upppercase names ok for struct names
// TODO read over gnu style
// TODO remove timeout info in tests
// TODO move raw socket functions to separate file?
// TODO remove unused imports/includes



/**
 * Main function to run compression detection for part 2.
 * Run with arg 1 as the path to the config file.
 * @param argc The number of args.
 * @param argv The pointer to the argv array.
 * @return Returns 0 if there are no errors, 1 otherwise.
 */
int
main(int argc, char *argv[])
{
  char *config_path = argv[1];
  if (!config_path)
    {
      printf ("Missing required arg config\n");
      return 1;
    }
  if (run_compdetect (config_path))
    {
      perror ("Error with run_compdetect");
      return 1;
    }
  return 0;
}

int
run_compdetect (char *config_path)
{
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
      config_destroy (config);
      return 1;
    }
  config_destroy (config);
  return 0;
}

